cmd_/home/shirin/Desktop/a2/q2/hider.mod := printf '%s\n'   hider.o | awk '!x[$$0]++ { print("/home/shirin/Desktop/a2/q2/"$$0) }' > /home/shirin/Desktop/a2/q2/hider.mod
