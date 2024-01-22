cmd_/home/shirin/Desktop/a2/q4/probe.mod := printf '%s\n'   probe.o | awk '!x[$$0]++ { print("/home/shirin/Desktop/a2/q4/"$$0) }' > /home/shirin/Desktop/a2/q4/probe.mod
