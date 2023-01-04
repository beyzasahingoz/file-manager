make: file_manager file_client

file_manager: file_manager.c
	 gcc file_manager.c -o file_manager -lreadline

file_client: file_client.c 
	gcc file_client.c -o file_client 

clear: 
	rm -rf *o shell