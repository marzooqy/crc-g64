from models import models
import subprocess
import time

print('| Model | 100 B | 1 KB | 10 KB | 100 KB | 1 MB | 10 MB | 100 MB |')

for name, model in models.items():
    if model.width >= 8:
        print('| ' + name, end=' ', flush=True)

        width = '-DWIDTH=' + str(model.width)
        poly = '-DPOLY=' + str(model.poly) + 'ULL'
        init = '-DINIT=' + str(model.init) + 'ULL'
        refin = '-DREFIN=' + str(model.refin).lower()
        refout = '-DREFOUT=' + str(model.refout).lower()
        xorout = '-DXOROUT=' + str(model.xorout) + 'ULL'

        compile = ['g++', '-c', width, poly, init, refin, refout, xorout, '-O3', 'bench.cpp', 'crc.cpp']
        link = ['g++', 'bench.o', 'crc.o', '-o', 'bench.exe']

        subprocess.run(compile).check_returncode()
        time.sleep(0.1)
        subprocess.run(link).check_returncode()
        time.sleep(0.1)
        subprocess.run(['bench.exe']).check_returncode()