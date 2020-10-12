# Compile options
VERBOSE=n
OPT=g
USE_NANO=y
SEMIHOST=n
USE_FPU=y

# Libraries
USE_LPCOPEN=y
USE_SAPI=n
USE_FREERTOS=y
FREERTOS_HEAP_TYPE=4
LOAD_INRAM=n

# Al no usar libreria SAPI:
# Peripheral addresses: chip_lpc43xx.h
# Basic chip functionality: chip.h
# Board initialization: board.h -> Hace falta modificarlo para organizarlo mejor. Board.h hace cosas que tiene que ver con la aplicación.