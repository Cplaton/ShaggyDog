#!/bin/bash
echo "Content-Type: text/html;charset=UTF-8"
echo
PATHDATA=`echo "$QUERY_STRING" | sed -n 's/^.*pathdata=\([^&]*\).*$/\1/p' | sed "s/%20/ /g" | sed "s/%2F/\//g"`
PATHMODELS=`echo "$QUERY_STRING" | sed -n 's/^.*pathmodels=\([^&]*\).*$/\1/p' | sed "s/%20/ /g"  | sed "s/%2F/\//g"`
CACHEMEM=`echo "$QUERY_STRING" | sed -n 's/^.*cachemem=\([^&]*\).*$/\1/p' | sed "s/%20/ /g"`
NPROCS=`echo "$QUERY_STRING" | sed -n 's/^.*nprocs=\([^&]*\).*$/\1/p' | sed "s/%20/ /g"`
echo
	
	echo "<html>"
	echo "<head>"
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"styleadmin.css\" />"	
	echo "<title>MSVMpack</title>"
	echo "</head>"
	echo "<body>"
	echo "<h1>MSVMpack Server</h1>"

echo $PATHDATA > server.conf
echo $PATHMODELS >> server.conf
echo $CACHEMEM >> server.conf
echo $NPROCS >> server.conf

echo "Settings saved."

echo "You can now go <a href=\"../home.cgi\">to MSVMpack server homepage</a>"
echo "or <a href=\"admin.cgi\">back to admin page</a>"
echo "<br><hr size=1 width=100% align=center color=#900000>"


	echo "</body></html>"

