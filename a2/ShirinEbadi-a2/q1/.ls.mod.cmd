cmd_/home/shirin/Desktop/a2/q1/ls.mod := printf '%s\n'   ls.o | awk '!x[$$0]++ { print("/home/shirin/Desktop/a2/q1/"$$0) }' > /home/shirin/Desktop/a2/q1/ls.mod
