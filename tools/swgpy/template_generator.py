# This file is part of SWGANH which is released under the MIT license.
# See file LICENSE or go to http://swganh.com/LICENSE
#
# Author : rcythr (Richard Laughlin)
# Description: Generates object templates from an output csv file

# At the time of creation, the SQL command used to generate the csv was:
# SELECT iff.iff_template, iff.object_type, 0, iff.attribute_template_id, o.objectName_1, o.objectName_2
# FROM iff_templates iff
# INNER JOIN objects o ON o.object_string = iff.iff_template
# WHERE iff.object_type != 0;
#
# INTO OUTFILE 'C:\\templates.csv'
# FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"'
# ESCAPED BY '\\'
# LINES TERMINATED BY '\n'
# 

import os, sys

CSV_FILENAME = "templates.csv"
TEMPLATE_FILENAME = "template_template.txt"

def main(force):
	#Load the template line by line
	template = []
	for line in open(TEMPLATE_FILENAME):
		template.append(line)
		
	saved_files = []
		
	#For every line in the csv
	for input in open(CSV_FILENAME):
		csv = []
		saved = False
		for i in map(lambda x: x.replace('\"', '').replace('\'', ''), input.strip().split(",")):
			csv.append(i)
	
		#Ensure the folder exists
		out_file =  '../../data/scripts/templates/' +csv[0]
		dir = os.path.dirname(out_file)
		if not os.path.exists(dir):
			os.makedirs(dir)
	
		#Convert entry at 0 to filename
		output_filename = "./" + out_file.replace(".iff", ".py")
		
		replaces = []
		if os.path.isfile(output_filename):
			replace = None
			#Read in preserved targets sections
			for line in open(output_filename):
				if not force and line.find(stop_condition) != -1:
					saved = True
					saved_files.append(output_filename)
					break
				elif line.find(preserved_replace[0]) != -1:
					replace = ""
				elif line.find(preserved_replace[1]) != -1:
					replaces.append(replace)
					replace = None
				elif replace is not None:
					replace += line
		
		if saved:
			continue
		
		#Open output file for writing
		with open(output_filename, 'w') as output:
			#print('GENERATING', output_filename)
			for line in template:
				if line.find(preserved_replace[0]) != -1:
					output.write(line)
					if len(replaces) > 0:
						output.write(replaces[0])
						replaces = replaces[1:]
					continue
				elif line.find(preserved_replace[1]) != -1:
					output.write(line)
					continue
				else:
					#Replace CSV Targets
					for target, index, func in csv_replace_targets:
						if line.find(target) != -1:
							line = line.replace(target, str(func(csv[index])))
					output.write(line)
					
	## Create __init__.py for each subfolder if doesn't exist
	for dirname, subdirs, filenames in os.walk('.'):
		if not os.path.isfile(dirname + os.sep + '__init__.py'):
			with open(dirname + os.sep + '__init__.py', 'w') as init_py:
				init_py.write('')
	
	if len(saved_files) > 0:
		print("The following files were saved. You must run with -f, or remove the MODIFIED TAG")
		for file in saved_files:
			print(file)
	
types = {
			'1112885583' : "Building",
			'1129465167' : "Creature",
			'1145850183' : "Guild",
			'1178818900' : "FactoryCrate",
			'1196578128' : "Group",
			'1229869903' : "Installation",
			'1230261839' : "Intangible",
			'1296651087' : "Mission",
			'1297302351' : "ManufactureSchematic",
			'1347174745' : "Player",
			'1380142671' : "ResourceContainer",
			'1396919380' : "Cell",
			'1398030671' : "Static",
			'1413566031' : "Tangible",
			'1463900496' : "Waypoint",
			'1789660414' : "Weapon",
			'1874303456' : "Ship"
		}
		
def objectType(id):
	return types[id]

def boolString(val):
	return val != "0"
	
csv_replace_targets = []
csv_replace_targets.append(("@MODEL_STRING@", 0, str))
csv_replace_targets.append(("@OBJECT_TYPE@", 1, objectType))
csv_replace_targets.append(("@IS_PROTOTYPE@", 2, boolString))
csv_replace_targets.append(("@ATTRIBUTE_ID@", 3, int))
csv_replace_targets.append(("@STF_FILE@", 4, str))
csv_replace_targets.append(("@STF_NAME@", 5, str))

preserved_replace = ("#### BEGIN MODIFICATIONS ####", "####  END MODIFICATIONS  ####")

stop_condition = "#### MODIFIED ####"

#Call to the main program
main((len(sys.argv) > 1 and sys.argv[1] == "-f"))