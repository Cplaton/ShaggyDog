#!/bin/bash

echo "Content-Type: text/html;charset=UTF-8"
echo
FILE=`echo "$QUERY_STRING" | sed -n 's/^.*file=\([^&]*\).*$/\1/p' | sed "s/%20/ /g"`
PATHDATA=`echo "$QUERY_STRING" | sed -n 's/^.*pathdata=\([^&]*\).*$/\1/p' | sed "s/%20/ /g"  | sed "s/%2F/\//g"`

	echo "<html>"
	echo "<head>"
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\" />"	
	echo "<title>MSVMpack</title>"
	echo "</head>"
	echo "<body>"
	echo "<h1>MSVMpack Server</h1>"

echo "<a href=\"home.cgi\">HOME</a> "
echo "<br><hr size=1 width=100% align=center>"

echo "<pre>"
./convert $PATHDATA$FILE $PATHDATA$FILE.mpd `cat $PATHDATA$FILE | wc -l` 4
echo "</pre>"

echo "</body></html>"


