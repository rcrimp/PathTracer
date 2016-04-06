#!BPY
"""Registration info for Blender menus:
Name: 'RCPT scene description (.txt)...'
Blender: 269
Group: 'Export'
Tooltip: 'Export for the RC Path Tracer scene file'
"""
##########################################################
# Blender exporter for RCPT raytracer scene files
##########################################################

bl_info = {
    "name": "RCPT export",
    "description": "Export for the RC Path Tracer scene file-2014",
    "author": "Raymond Scurr, David Eyers, Reuben Crimp",
    "version": (1, 0),
    "blender": (2, 65, 0),
    "location": "View3D > File > Export",
    "warning": "Bugs ahoy!",
    "wiki_url": "http://wiki.blender.org/index.php/Extensions:2.5/Py/"
                "Scripts/My_Script",
    "category": "Import-Export"}

####################################
# Global Variables
####################################

# If false, will overwrite files without asking.
# ... not implemented in this 2.6x plugin yet
_safeOverwrite = False 

# Private, don't change these
_file = None

####################################
# Library dependancies
####################################
import bpy
from bpy.props import *

import mathutils, math, struct
import os
import time
import bpy_extras
from bpy_extras.io_utils import ExportHelper 
import shutil

from bpy.types import World, Camera, Object, Mesh, MetaBall, Material, Lamp

##########################################################
# Functions for writing output file
##########################################################
def export_file(context,filename):
  global _file
  _file = open(filename,"w")
  
  print("")
  print("*** Exporting scene file to:")
  print(" ", filename)
  writeFile(context,_file)
  _file.close()
  print("*** Done")

########################################################
# Functions for writing scene structure
########################################################

def writeFile(context,file):
  # World details
  writeWorld(context)

  # Three loop structure just for logic simplicity.
  # Could produce object groups more nicely...

  # Get the scene objects
  objects = context.scene.objects
  
  # Camera
  for obj in objects:
    if obj.type == "CAMERA":
      writeCamera(obj)

  # Objects
  _file.write("\n\n")
  _file.write("# Outputting the objects in the scene\n")
  _file.write("# NOTE: Converted all Meshes as spheres\n")
  for obj in objects:
    if obj.type == "MESH" :
      print("Exporting Object : \"%s\" " % (obj.name))
      _file.write("\n# Exported Object : \"%s\" \n" % (obj.name))
      _file.write("sphere\n")
      writeMaterialProperties(obj.material_slots)
      writeTransformation(obj)
      
  # Done!
  _file.write("\nendview\n")


#############################################################
# Functions for exporting Objects
#############################################################
def writeMaterialProperties(materials):
  if (len(materials) > 1):
    print("")
    print("Warning: Object has more than one material.")
    print("Only one material property will be output!!")
    print("")
    _file.write("\n# Multiple Materials not exported.\n")
  if (len(materials) == 0):
    writeDefaultMaterial()
  else:
    writeMaterial(materials[0].material)


############################################################
# Functions for exporting material data
############################################################

# Exporting default material
def writeDefaultMaterial():
  _file.write("ambient   0.500 0.500 0.500\n")
  _file.write("diffuse   0.640 0.640 0.640\n")
  _file.write("specular  0.500 0.500 0.500  50.00\n")
  _file.write("mirror    0.000 0.000 0.000\n")
  _file.write("texture   0\n")

# Exporting material
def writeMaterial(mat):
  # should try to better match past exporters' output
  amb = min(float(mat.ambient),1.0)
  _file.write("ambient   %.3f %.3f %.3f\n" % (amb,amb,amb))
  _file.write("diffuse   %.3f %.3f %.3f\n" % scaleColour(mat.diffuse_color,mat.diffuse_intensity))
  _file.write("specular  %.3f %.3f %.3f  " % scaleColour(mat.specular_color,mat.specular_intensity))
  _file.write("%.3f\n" % (mat.specular_hardness))
  _file.write("mirror    %.3f %.3f %.3f\n" % scaleColour(mat.mirror_color,mat.raytrace_mirror.reflect_factor))
  _file.write("texture   0\n")

def scaleColour(c,s):
  return tuple([v*s for v in c])


#########################################################
# Functions for exporting details about the World
#########################################################
def writeWorld(context):
  print("Exporting the World")

  ## Some hard coded values for the view screen
  ## Rather dodgy, and may need to be fixed!!!
  ##
  _file.write("\n")
  _file.write("# default value for the size of the image to be created\n")
  _file.write("imagesize     320 \n")
  _file.write("\n")

  ## An attempt to get some information about the world
  ## Background == Horizon
  _file.write("# The background colour of the scene\n")
  _file.write("# -- using the blender 'horizon' value\n")
  hor = context.scene.world.horizon_color
  _file.write("background    %.3f %.3f %.3f\n" % tuple(hor) )
  _file.write("\n")

  ## Get some information about the world
  _file.write("# The ambient lighting of the scene\n")
  amb = tuple([min(c*2.0,1.0) for c in tuple(context.scene.world.ambient_color)])
  _file.write("ambient       %.3f %.3f %.3f\n" % amb)
  _file.write("\n")
  
  
#########################################################
# Functions for exporting camera data
#########################################################
def writeCamera(obj):
  print("Exporting Camera : \"%s\"" % (obj.name))

  _file.write("# the default camera is located at the origin\n")
  _file.write("# the default viewplane has corners\n")
  _file.write("# --  top left:     (-16,  16, -lens) \n")
  _file.write("# --  bottom right: ( 16, -16, -lens) \n")
  _file.write("# ((Actually, Blender uses a 32*24 viewplane))\n")
  _file.write("# \n")

  ## The camera's 'lens'  -- distance to standard viewplane (32 * 24)
  cam = obj.data.lens
  _file.write("lens  %.3f\n" % (cam) )

  ## Output the camera transformation
  writeTransformation(obj)


#########################################################
# Functions for exporting light data
#########################################################
def writeLamp(obj):
  print("Exporting Light : \"%s\"" % (obj.name))
  # output all blender light sources as point lights
  writePointLight(obj)

def writePointLight(obj):
  _file.write("light   ")
  _file.write("%.3f %.3f %.3f   "   % tuple(obj.location) )
  _file.write("%.3f %.3f %.3f\n"    % tuple(obj.data.color) )


#########################################################
# Functions for exporting transformations
#########################################################
def writeTransformation(obj):
  ## Output the object transformation from the 'standard position'
  if tuple(obj.scale) != (1, 1, 1) :
    _file.write("stretch    %.3f %.3f %.3f\n" % tuple(obj.scale) )
  if obj.rotation_euler[0] != 0 :
    _file.write("rotate     x  %.3f\n" % math.degrees(obj.rotation_euler[0]) )
  if obj.rotation_euler[1] != 0 : 
    _file.write("rotate     y  %.3f\n" % math.degrees(obj.rotation_euler[1]) )
  if obj.rotation_euler[2] != 0 :
    _file.write("rotate     z  %.3f\n" % math.degrees(obj.rotation_euler[2]) )
  (x,y,z) = obj.location  
  if (x,y,z) != (0, 0, 0) :
    _file.write("translate  %.3f %.3f %.3f\n" % (x,y,z) )

##########################################################
# Export operator
##########################################################

class Export_RCPTtxt(bpy.types.Operator, ExportHelper):
    '''Exports scene as RCPT scene file.'''
    bl_idname = "export_object.txt"
    bl_label = "Export RCPT file (.txt)"

    filename_ext = ".txt"

    def execute(self, context):
#        props = self.properties
        filepath = self.filepath
        filepath = bpy.path.ensure_ext(filepath, self.filename_ext)

        exported = export_file(context,filepath)

        return {'FINISHED'}

    def invoke(self, context, event):
        wm = context.window_manager

        if True:
            # File selector
            wm.fileselect_add(self) # will run self.execute()
            return {'RUNNING_MODAL'}
        elif True:
            # search the enum
            wm.invoke_search_popup(self)
            return {'RUNNING_MODAL'}
        elif False:
            # Redo popup
            return wm.invoke_props_popup(self, event) #
        elif False:
            return self.execute(context)

##########################################################
# Blender API stuff
##########################################################

def menu_func(self, context):
    self.layout.operator(Export_RCPTtxt.bl_idname, text="RCPT file (.txt)")

def register():
    bpy.utils.register_module(__name__)
    bpy.types.INFO_MT_file_export.append(menu_func)
     
def unregister():
    bpy.utils.unregister_module(__name__)
    bpy.types.INFO_MT_file_export.remove(menu_func)

if __name__ == "__main__":
    register()
