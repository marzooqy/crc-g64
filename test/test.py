from models import models
import subprocess
import time

failed = False

with open('../crc.cpp', 'r') as file:
    code = file.read()

start = code.find('#define N ') + 10
end = start + code[start:].find('\n')

N = int(code[start:end])

for name, model in models.items():
    if model.width >= N:
        print(name, flush=True)

        width = '-DWIDTH=' + str(model.width)
        poly = '-DPOLY=' + str(model.poly) + 'ULL'
        init = '-DINIT=' + str(model.init) + 'ULL'
        refin = '-DREFIN=' + str(model.refin).lower()
        refout = '-DREFOUT=' + str(model.refout).lower()
        xorout = '-DXOROUT=' + str(model.xorout) + 'ULL'
        check = '-DCHECK=' + str(model.check) + 'ULL'

        compile = ['g++', '-c', width, poly, init, refin, refout, xorout, check, '-O3', 'test.cpp', '../crc.cpp']
        link = ['g++', 'test.o', 'crc.o', '-o', 'test.exe']

        print(' '.join(compile), flush=True)
        subprocess.run(compile).check_returncode()
        time.sleep(0.1)
        subprocess.run(link).check_returncode()
        time.sleep(0.1)

        exit_code = subprocess.run(['./test.exe']).returncode

        if not failed and exit_code != 0:
            failed = True

        print()

if failed:
    raise Exception('Test failed')
else:
    print('The test ran successfully!')