NIC = eno2

up: ns-up sdplane-up

ns-up:
	sudo ./ns-up.sh $(NIC)

sdplane-up:
	sudo ../../sdplane/sdplane -f ./sdplane.conf 

conf:
	sudo ./router-conf.sh

down: ns-down

ns-down:
	sudo ./ns-down.sh
