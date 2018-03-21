For running the programs follow these steps:

For UDP:

Server:
After compiling use this command type:
./complied_name port_number
Example:
./a_s 15010


Client:
After compiling use this command type:
./complied_name  server_IP port_number connection_type file_name
Example:
./a_c 149.160.202.17 15010 non-persistent text_1MB.txt
You can always use non-persistent connection type since UDP is connectionless!
NOTE: IF data is lost in the network, Client will not terminate the loop and will not give any number as output. 
So you may need to run it couple of times to make sure no data is lost.


For TCP

Server:
After compiling use this command type:
./complied_name port_number
Example:
./a_s 15010
To stop the server you should kill the process or change the port number.

Client:
After compiling use this command type:
./complied_name  server_IP port_number connection_type file_name number_of_files
Example:
./a_c 149.160.202.17 15010 non-persistent text_1MB.txt 10



