import telnetlib
tn = telnetlib.Telnet('127.0.0.1', 9999)
tn.mt_interact()