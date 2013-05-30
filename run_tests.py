import subprocess
from os import listdir
from os.path import isfile

test_configs = [ f for f in (f for f in listdir(".") if isfile(f)) if ".json" == f[-5:]]

errors = list()
for f in test_configs:
#  try:
#    subprocess.check_output(["./transitionTest",f],stderr=subprocess.STDOUT)
#  except subprocess.CalledProcessError,e:
#    print f,"failed\n",e.output
  print "**********",f,"*********"
  try:
    subprocess.check_call(["./transitionTest",f],stderr=subprocess.STDOUT)
  except subprocess.CalledProcessError,e:
    print f,"failed\n",e.returncode
    errors.append([f,e.returncode])

print "OK",len(test_configs)-len(errors)

if errors:
  print "ERROR",len(errors)
  for f,code in errors:
    print f,code

    