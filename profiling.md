In release mode:

test_short.wav (5 seconds):
```
$ bin/Release/MUSI6106Exec test_short.wav verb.wav 0.01 time
reading/writing done in:        0.178219 seconds.
time in process/flushBuffer:    0.17361 seconds.

$ bin/Release/MUSI6106Exec test_short.wav verb.wav 0.01 freq
reading/writing done in:        0.020984 seconds.
time in process/flushBuffer:    0.015274 seconds.

$ bin/Release/MUSI6106Exec test_short.wav verb.wav 0.1 time
reading/writing done in:        1.59182 seconds.
time in process/flushBuffer:    1.5865 seconds.

$ bin/Release/MUSI6106Exec test_short.wav verb.wav 0.1 freq
reading/writing done in:        0.022417 seconds.
time in process/flushBuffer:    0.01609 seconds.

$ bin/Release/MUSI6106Exec test_short.wav verb.wav 1.0 time
reading/writing done in:        17.2262 seconds.
time in process/flushBuffer:    17.2169 seconds.

$ bin/Release/MUSI6106Exec test_short.wav verb.wav 1.0 freq
reading/writing done in:        0.030916 seconds.
time in process/flushBuffer:    0.022162 seconds.
```

test.wav (3 minutes and 13 seconds):
```
$ bin/Release/MUSI6106Exec test.wav verb.wav 0.001 time
reading/writing done in:        0.740095 seconds.
time in process/flushBuffer:    0.675906 seconds.

$ bin/Release/MUSI6106Exec test.wav verb.wav 0.001 freq
reading/writing done in:        0.407628 seconds.
time in process/flushBuffer:    0.331456 seconds.

$ bin/Release/MUSI6106Exec test.wav verb.wav 0.01 time
reading/writing done in:        6.40588 seconds.
time in process/flushBuffer:    6.33339 seconds.

$ bin/Release/MUSI6106Exec test.wav verb.wav 0.01 freq
reading/writing done in:        0.346847 seconds.
time in process/flushBuffer:    0.279686 seconds.

# Further time-domain tests omitted because they take longer than I care to wait.

$ bin/Release/MUSI6106Exec test.wav verb.wav 0.1 freq
reading/writing done in:        0.353868 seconds.
time in process/flushBuffer:    0.288613 seconds.

$ bin/Release/MUSI6106Exec test.wav verb.wav 1.0 freq
reading/writing done in:        0.52992 seconds.
time in process/flushBuffer:    0.454823 seconds.
```
