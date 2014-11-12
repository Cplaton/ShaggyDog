#!/bin/bash

echo "Content-Type: text/html;charset=UTF-8"
echo
TRAINFILE=`echo "$QUERY_STRING" | sed -n 's/^.*trainfile=\([^&]*\).*$/\1/p' | sed "s/%20/ /g"`
MODELFILE=`echo "$QUERY_STRING" | sed -n 's/^.*modelfile=\([^&]*\).*$/\1/p' | sed "s/%20/ /g"`
PATHMODELS=`echo "$QUERY_STRING" | sed -n 's/^.*pathmodels=\([^&]*\).*$/\1/p' | sed "s/%20/ /g"  | sed "s/%2F/\//g"`

ps -Ao args > .ps

if cat .ps | grep trainmsvm | grep $PATHMODELS$MODELFILE > /dev/null
then 
	URL="result.cgi?trainfile=$TRAINFILE&modelfile=$MODELFILE&pathmodels=$PATHMODELS"
	DONE="no"
else
	DONE="yes"
	URL="stop.cgi?trainfile=$TRAINFILE&modelfile=$MODELFILE&pathmodels=$PATHMODELS&done=$DONE"
fi

	echo "<html>"
	echo "<head>"
	echo "<meta http-equiv=\"refresh\" content=\"2; URL=$URL\">"
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\" />"	
	echo "<title>MSVMpack</title>"
	echo "</head>"
	echo "<body>"
	echo "<h1>MSVMpack Server</h1>"

echo "Running on $(hostname -s)<br>"
echo "Training $MODELFILE on data file: $TRAINFILE<br>" 
echo "<a href=\"stop.cgi?trainfile=$TRAINFILE&modelfile=$MODELFILE&pathmodels=$PATHMODELS&done=$DONE\">STOP</a> "
echo "<br><hr size=1 width=100% align=center>"
echo "<pre>"
echo "$(cat $MODELFILE.training)"
echo "</pre>"

echo "</body></html>"

