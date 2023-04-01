# cacheSim

A way to simulate cache replacement policies and analyze their pattern behavior

Inputs:
- small cache config csvs with necessary parameters for a run
- run exe with directory of configs as input, inputs/\${directoryName}/

Outputs:
- name of folder based on seconds
- outputs/\${time-in-seconds}/\${programName-repPolicy-A#-B#-C#_timestamp}.csv