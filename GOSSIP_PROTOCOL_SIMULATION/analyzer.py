"""
**********************

Progam Name: ECE428 MP1 Analysis Tool

Code authors: Varun Badrinath Krishna

About this file: Member Node Implementation and Failure Detection using the SWIM Protocol

The MIT License (MIT)

Copyright (c) 2014 Varun Badrinath Krishna

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

**********************/
"""

import sys
import os
import subprocess

def analyze_log(logfile):
	ifile = open(logfile,"r")
	joins_info = {}
	failed_nodes = []
	fails_info = {}
	accurancy_issues = {}

	global program_run
	print "Analyzing..."
	
	import time
	start_time = time.time()

	global show_logs
	print_inputs = show_logs

	global N
	for line in ifile:
		stripped_line = line.strip()
		words = stripped_line.split(' ') #shlex.split(stripped_line);
		#print words
		if len(words) < 2:
			continue
		speaker_node_parts = words[0].split('.')
		if len(speaker_node_parts) <= 2:
			continue
		
		if '[' not in words[1]:
			continue


		time_txt = words[1]
		time_stamp = int(time_txt[1:-1])
		
		speaker_node = int(speaker_node_parts[0])
		if speaker_node not in joins_info:
			joins_info[speaker_node]=[]

		if speaker_node not in fails_info:
			fails_info[speaker_node]=[]


		if "Node failed" in line:
			failed_nodes.append([speaker_node,time_stamp])
			if print_inputs:
				print line.strip()

		else:
			for i in range(1,N+1):
				s = "Node %d.0.0.0:0 joined"%i
				t = "Node %d.0.0.0:0 removed"%i
				if s in line:
					joins_info[speaker_node].append([i,time_stamp])
					if print_inputs:
						print line.strip()
				elif t in line:
					fails_info[speaker_node].append([i,time_stamp])
					if print_inputs:
						print line.strip()




	join_results = []
	fail_results = []
	accuracy_results = []

	join_success = []
	fail_success = []


	for speaker_node in joins_info:
		for i in range(1,N+1):
			join_counts = [val for val in joins_info[speaker_node] if val[0] == i]
			if len(join_counts) == 0:
				join_results.append("Error: Node%d did not add Node%d"%(speaker_node,i))
			elif len(join_counts) > 1:
				join_results.append("Error: Node%d added Node%d %d times at times: "%(speaker_node,i,len(join_counts)) + [val[1] for val in join_counts])
			else:
				join_success.append("Success: Node%d added Node%d"%(speaker_node,i))
			
		if speaker_node in fails_info:
			for i in range(1,N+1):
				#get all items in fails_info[speaker_node] if the item pertains to index i
				fail_counts = [val for val in fails_info[speaker_node] if val[0] == i]

				#if the node i has failed, then failed_item_i will contain node i. If not, failed_item_i is empty
				failed_item_i = [val for val in failed_nodes if val[0] == i]
				
				#if the node i has failed, then failed_item_i will contain node i. If not, failed_item_i is empty
				failed_item_speaker = [val for val in failed_nodes if val[0] == speaker_node]
				
				if len(failed_item_i) == 1: #Node i has failed
					if len(fail_counts) == 1 and failed_item_i[0][0] == speaker_node: #Speaker node should not remove itself
						fail_results.append("Error: Node%d removed itself"%(speaker_node))
					elif len(fail_counts) == 0:
						if failed_item_i[0][0] == speaker_node:
							fail_success.append("Success: Node%d did not remove itself"%(speaker_node))
						elif len(failed_item_speaker) == 1: # and failed_item_speaker[0][1] < failed_item_i[0][1]:
							fail_success.append("Success: Node%d did not remove Node%d when it failed first"%(speaker_node,i))
						else:
							fail_results.append("Error: Node%d did not remove Node%d"%(speaker_node,i))
					elif len(fail_counts) > 1:
						fail_results.append("Error: Node%d removed Node%d %d times at times: "%(speaker_node,i,len(fail_counts)) + [val[1] for val in fail_counts])
					elif fail_counts[0][1] < failed_item_i[0][1]: #if detected 1, must be that the detection happened after the failure
							accuracy_results.append("Error: Node%d removed Node%d before Node%d actually failed"%(speaker_node,i,i))
					else:
						fail_success.append("Success: Node%d removed Node%d on time!"%(speaker_node,i))
		
				elif len(fail_counts) > 0: #Must detect 0
						accuracy_results.append("Error: Node%d removed Node%d though Node%d did not fail"%(speaker_node,i,i))

	global errors_only
	global hr
	print "Analysis completed in %.2f seconds"%(time.time()-start_time)
	print hr
	print "JOIN RESULTS : " + ("Errors were found" if len(join_results) > 0 else "All good!")
	for result in join_results:
		print result
	if not errors_only:
		for result in join_success:
			print result

	print hr
	print "COMPLETENESS RESULTS : " + ("Errors were found" if len(fail_results) > 0 else "All good!")
	for result in fail_results:
		print result

	if not errors_only:
		for result in fail_success:
			print result

	print hr
	print "ACCURACY RESULTS : " + ("Errors were found" if len(accuracy_results) > 0 else "All good!")
	for result in accuracy_results:
		print result

	if len(join_results) > 0 or len(fail_results) > 0 or len(accuracy_results) > 0:
		print "The last program run failed. The previous %d program runs succeeded"%(program_run)
		quit()


N = 10
hr = "-----------------"
try:
	config_file = sys.argv[1]
	cf_handler = open(config_file,"r")
	for line in cf_handler:
		words = line.split(' ')
		if len(words) != 2:
			continue
		if "MAX_NNB" in words[0]:
			N = int(words[1])
	cf_handler.close()
except:
	print "Please provide a valid test case config file as the first parameter"

print "Number of nodes: %d"%N

logfile = "dbg.log"#sys.argv[1]
with open(os.devnull, "w") as fnull:
	subprocess.check_call("make",stdout=fnull)

try:
	repetitions = int(sys.argv[2])
	print "Repetitions: %d"%repetitions
except:
	repetitions = 1
	print "Repetitions (default): %d"%repetitions

errors_only = True
if "-a" in sys.argv:
	errors_only = False

show_logs = True
if "-h" in sys.argv:
	show_logs = False

print "Only errors will be displayed. Use -a to show all." if errors_only else "All messages will be displayed"

program_run = 0

for program_run in range(0,repetitions):
	#Overwrite log file
	logfile_handler = open(logfile,"w")
	logfile_handler.close()
	
	print hr
	print "Program run %d"%(program_run+1)
	hr = "-----------------"
	import time
	start_time = time.time()
	with open(os.devnull, "w") as fnull:
		subprocess.check_call("./app %s"%config_file,shell=True,stdout=fnull)
	print "Run completed. Run time: %.2f seconds"%(time.time() - start_time)
	analyze_log(logfile)
	
