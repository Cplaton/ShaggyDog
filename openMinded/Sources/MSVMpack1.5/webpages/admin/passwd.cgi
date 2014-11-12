#!/bin/bash
echo "Content-Type: text/html;charset=UTF-8"
echo
PASSWD=`echo "$QUERY_STRING" | sed -n 's/^.*passwd=\([^&]*\).*$/\1/p' | sed "s/%20/ /g"`
echo
	
	echo "<html>"
	echo "<head>"
#	echo "<meta http-equiv=\"refresh\" content=\"2; URL=$URL\">"
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"styleadmin.css\" />"	
	echo "<title>MSVMpack</title>"
	echo "</head>"
	echo "<body>"
	echo "<h1>MSVMpack Server</h1>"
	
mongoose -A .htpasswd "mydomain.com" "admin" "$PASSWD"

echo "Password changed."

echo "<a href=\"admin.cgi\">Back to admin page</a>"
echo "<br><hr size=1 width=100% align=center>"


	echo "</body></html>"

