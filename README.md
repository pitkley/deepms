# deepms

Some people, including me, have the issue that some monitors connected via DisplayPort to a Nvidia graphics card under Linux will not honor DPMS signals and thus not turn off ([1][dpms-dp-nvidia], [2][dpms-dp-askubuntu]).
To not impair the lifetime of a single monitor of mine too much, I have written this tool as a work-around.

`deepms` will regularly (currently every two seconds) check, if DPMS has been activated and the monitors should turn off.
It will then use [`libddccontrol`][libddccontrol] to send a DPMS-standby-signal to every monitor that isn't already in standby.
As soon as DPMS deactivates, it will send the DPMS-on-signal to turn them back on.

Your mileage may vary greatly, since DDC/CI seems to be hit-and-miss if it's working or not.
DDC/CI needs to be supported by your graphics card (modern ones usually do) and by your monitors.
While it is working for me over DVI, HDMI and (crucially) DisplayPort, it might not work for you.

*Notice:* This requires that you have ddccontrol installed and working.
Furthermore, you will probably have to change the permissions on the I2C-devices so that you can run `deepms` without being root.
See [here][ddccontrol-issue-5] and [here][gddccontrol-non-root] on how you can adapt the permissions.


## License

This software is under the MIT license.


[dpms-dp-nvidia]: https://devtalk.nvidia.com/default/topic/791786/dpms-not-working-on-gtx980-with-displayport-connection/
[dpms-dp-askubuntu]: https://askubuntu.com/questions/546818/displayport-monitor-not-detected-if-switched-off-and-on-again
[libddccontrol]: https://github.com/ddccontrol/ddccontrol
[ddccontrol-issue-5]: https://github.com/ddccontrol/ddccontrol/issues/5
[gddccontrol-non-root]: http://www.techytalk.info/debian-ubuntu-gddccontrol-non-root/
