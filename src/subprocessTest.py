from subprocess import Popen, PIPE

proc = Popen("./TheSolvers", stdin = PIPE, stdout = PIPE, stderr = PIPE)

while True:
    a = input() 

    print("|" + a + "|")

    proc.stdin.write(bytes(a + "\n", encoding = "utf-8"))
    proc.stdin.flush()


    result = proc.stdout.readline().decode("utf-8")
    print(result)



