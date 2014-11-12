#!/bin/bash

echo "Content-Type: text/html;charset=UTF-8"
echo

TRAINFILE=`echo "$QUERY_STRING" | sed -n 's/^.*trainfile=\([^&]*\).*$/\1/p' | sed "s/%20/ /g"`
MODELFILE=`echo "$QUERY_STRING" | sed -n 's/^.*modelfile=\([^&]*\).*$/\1/p' | sed "s/%20/ /g"`
PATHMODELS=`echo "$QUERY_STRING" | sed -n 's/^.*pathmodels=\([^&]*\).*$/\1/p' | sed "s/%20/ /g"  | sed "s/%2F/\//g"`
DONE=`echo "$QUERY_STRING" | sed -n 's/^.*done=\([^&]*\).*$/\1/p'`

	echo "<html>"
	echo "<head>"
#	echo "<meta http-equiv=\"refresh\" content=\"2; URL=Server.cgi?on=0\">"
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\" />"
	echo "<title>MSVMpack Server</title>"
	echo "</head>"
	echo "<body>"
	echo "<h1>MSVMpack Server</h1>"
	
if [ "$DONE" == "no" ]
then
pkill -f $MODELFILE
fi

echo "Training stopped."

echo "<a href=\"home.cgi\">Back to homepage</a>"
if [ "$DONE" == "yes" ]
then
echo "or download the trained model: <a href=\"$PATHMODELS$MODELFILE\">$MODELFILE</a> "
fi
echo "<br><hr size=1 width=100% align=center>"
echo "<pre>"
echo "$(cat $MODELFILE.training)"
echo "</pre>"

rm $MODELFILE.training

	echo "</body></html>"

