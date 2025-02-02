import bpy
import xml.etree.ElementTree as ET
from mathutils import Euler, Vector

blend_file_directory = bpy.path.abspath('//')

def import_obj(file_path, position, rotation):
	bpy.ops.import_scene.obj(filepath=file_path)
	obj = bpy.context.selected_objects[0]

	# Set object's position
	obj.location = Vector([float(val) for val in position.split()])

	# Convert degrees to radians
	rotation_degrees = [float(angle) for angle in rotation.split()]
	rotation_radians = [angle * (3.14159 / 180) for angle in rotation_degrees]

	# Apply rotations
	obj.rotation_euler = Euler(rotation_radians)


# Parse the XML file
FOLDER = "./test/"
xml_file = FOLDER + "scene.xml"
tree = ET.parse(xml_file)
root = tree.getroot()

i = 0
n = 0 # Change by 100 for next batch

# Loop through the <mesh> elements in the XML
for mesh in root.findall('mesh'):
	i = i + 1
	if i <= n:
		continue

	file_path = FOLDER + mesh.get('file')
	position = mesh.get('pos')
	rotation = mesh.get('rot')
	print("FILE PATH:", file_path)
	
	# Import the OBJ file and set properties
	import_obj(file_path, position, rotation)
	if i % 100 == 0:
		break

# To rotate the scene press:
# 'A' 'R' 'X' '9' '0' 'Enter'

# To set texture:
# Go to Material Properties (2nd Red World Icon at the bottom right panel) -> New -> Base Color (Yellow Dot) -> Image Texture -> Open
# Select palette.png
# Press Ctrl + 'L' -> Link Materials
# Click on icon Viewport Shading (3rd World Icon in view)
