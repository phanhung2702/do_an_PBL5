import mysql.connector
import json
import datetime
import wx
from playsound import playsound
defaut_config = '{"server":{"host":"192.168.1.1","user":"phanhung", "password":30, "database":"New York"},"local":{"host":"localhost","user":"user", "password":27022003, "database":"New York"},"camera":{"name":"noname","id":0}}'
config = json.loads(defaut_config)
try: 
  mydb = mysql.connector.connect(
    host=config["local"]["host"],
    user=config["local"]["user"],
    passwd=str(config["local"]["password"]))
except:
  print("can not connect to server")
  exit()
print(mydb)
mycursor = mydb.cursor()
print("connect localhost")
"""
sql_insert = "INSERT INTO `detect_person`.`history` (`id`, `name`,`time_in`, `time_out`) VALUES (%s, %s , %s, %s)"
val_insert = ("e6184", "phanhung","2024-06-12 12:39:50",None)
try:
	mycursor.execute(sql_insert,val_insert)
	mydb.commit()
	print(mycursor.rowcount, "record inserted.")
except:
	print "error insert"
"""
x = datetime.datetime.now()
print(x.strftime('%Y-%m-%d %H:%M:%S'))
playsound("")