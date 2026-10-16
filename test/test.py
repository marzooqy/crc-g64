from models import models
import subprocess
import time

failed = False

for name, model in models.items():
    print(name)

    width = '-DWIDTH=' + str(model.width)
    poly = '-DPOLY=' + str(model.poly) + 'ULL'
    init = '-DINIT=' + str(model.init) + 'ULL'
    refin = '-DREFIN=' + str(model.refin).lower()
    refout = '-DREFOUT=' + str(model.refout).lower()
    xorout = '-DXOROUT=' + str(model.xorout) + 'ULL'
    check = '-DCHECK=' + str(model.check) + 'ULL'

    compile = ['g++', '-c', width, poly, init, refin, refout, xorout, check, 'test.cpp', '../crc.cpp']
    link = ['g++', 'test.o', 'crc.o', '-o', 'test.exe']

    print(' '.join(compile))
    subprocess.run(compile).check_returncode()
    time.sleep(0.1)
    subprocess.run(link).check_returncode()
    time.sleep(0.1)

    exit_code = subprocess.run(['test.exe']).returncode

    if not failed and exit_code != 0:
        failed = True

    print()

if failed:
    raise Exception('Test failed')
else:
    print('The test ran successfully!')