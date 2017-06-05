ALL: mftp mftp_client

.PHONY: ALL

mftp:
	gcc mftp.c -o2 -o mftp
	
mftp_client:
	gcc mftp_client.c -o2 -o mftp_client
	
clean:
	rm mftp
	rm mftp_client
	rm tmp.data
