#!/bin/bash

echo "Content-Type: text/html;charset=UTF-8"
echo
TESTFILE=`echo "$QUERY_STRING" | sed -n 's/^.*testfile=\([^&]*\).*$/\1/p' | sed "s/%20/ /g"`
PATHDATA=`echo "$QUERY_STRING" | sed -n 's/^.*pathdata=\([^&]*\).*$/\1/p' | sed "s/%20/ /g" | sed "s/%2F/\//g"`
MODELFILE=`echo "$QUERY_STRING" | sed -n 's/^.*modeltest=\([^&]*\).*$/\1/p' | sed "s/%20/ /g"`
PATHMODELS=`echo "$QUERY_STRING" | sed -n 's/^.*pathmodels=\([^&]*\).*$/\1/p' | sed "s/%20/ /g"  | sed "s/%2F/\//g"`
OUTFILE=pred.outputs



	echo "<html>"
	echo "<head>"
#	echo "<meta http-equiv=\"refresh\" content=\"2; URL=$URL\">"
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\" />"	
	echo "<title>MSVMpack</title>"
	echo "</head>"
	echo "<body>"
	echo "<h1>MSVMpack Server</h1>"

echo "Testing $MODELFILE on data file $TESTFILE<br>" 
echo "<a href=\"home.cgi\">Back to homepage</a>"
echo "or download the <a href=\"$OUTFILE\">output file</a> "
echo "<br><hr size=1 width=100% align=center>"
echo "<pre>"
./predmsvm $PATHDATA$TESTFILE $PATHMODELS$MODELFILE $OUTFILE
echo "</pre>"

echo "</body></html>"

