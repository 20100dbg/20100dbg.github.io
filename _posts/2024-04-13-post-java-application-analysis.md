---
title: "Get that DB access : a Java application analysis"
categories:
  - Reverse
tags:
  - Gentle Reverse Engineering
  - MySQL
  - Java
---


At work, we use an internal software from another department.
This software receives, processes and store data into a local MySQL database ; this data in itself is very useful, but the software does not do much with it and does not allow you to retrieve it so you would be able process it as you wish.
Alas, the department in charge of this software won't providing even a restricted access to the MySQL database.

From this frustration, a challenge was born : finding read access to this MySQL database and finally be able to fully exploit that data.
Fortunately, everything is on my side : I have this installed and working Java software, its complete setup, and admin access to the machine hosting both this software and database.


Let's try and find that MySQL password !


### Option 0 : Password reset

First thing that comes in mind would to Google "reset MySQL root password".

This method (detailled below) allows one to define a new root password, thus getting complete control over this MySQL instance.
Once we got root, we can of course access all needed data, but also create a new user we control, and even restore root's original hash password in case the app is using this account or just if we want to remain stealth.

But this is not ideal : we need to do this on each standalone computer, this setup is not noob-friendly, and ... too easy ? For the challenge I wanted to go deeper.


##### Detailled instructions
So our goal here is to takeover the MySQL server, create our own user, and restore root's original password (remember, we don't want to break our software's database access and we want to remain as stealth as possible)

```
#Start mysqld with :
mysqld --skip-grant-tables

#Connect as root without password
mysql -u root

#Save current root's hash (don't forget leading *)
select host, user, password from mysql.user;
```

![Root password](/assets/images/post-20240413/0_mysql_root_pass.png)

```
#Ok now log out, and stop mysqld

#Create a file at C:\mysql-init.txt containing :
UPDATE mysql.user SET password=PASSWORD('MyNewPass') WHERE user='root';
FLUSH PRIVILEGES;

#Start mysql and execute our script at startup :
mysqld --init-file=C:\mysql-init.txt

#We now can login using these credentials
mysql -u root -pMyNewPass

#Create our own privileged user
CREATE USER '20100dbg'@'localhost' IDENTIFIED BY '20100dbg';
GRANT ALL ON *.* TO '20100dbg'@'localhost' WITH GRANT OPTION;

#And restore root's original password
UPDATE mysql.user SET password='<HASH>', authentication_string=NULL WHERE user='root';
FLUSH PRIVILEGES;
```

And we're finally done.



- [Reset MySQL root's password](http://doc.docs.sk/mysql-refman-5.5/resetting-permissions.html)



### Option 1 : Extracting the installer

It turns out that it is possible to recover the password without even installing the application.
The app's installer takes care of installing and configuring the MySQL database (creating a user, setting passwords, etc.). But to perform these tasks, the installer needs to connect to Mysql, right ? And so use credentials, probably root's one ?

Several tools exist for decompressing setups, but InnoExtractor was the most useful. After dissecting the executable, we find icons, scripts, a binary and an extraction of this binary, a file called CodeSection.txt

![Extracted setup files](/assets/images/post-20240413/1_extract_files.png)


In this file we find several SQL queries, and we focus on one that should set a password with `IDENTIFIED BY`. No password here yet, but it looks like GlobalVar[7] and GlobalVar[8] could store a password.

![Extracted setup script 2](/assets/images/post-20240413/1_extract_script_2.png)

And bingo, at the end of this file, we find these variables being set with strings looking like passwords.

![Extracted setup script](/assets/images/post-20240413/1_extract_script.png)


- [InnoExtractor homepage](http://havysoft.cl)


### Option 2 : Interception of the configuration script

By monitoring file system activity with ProcMon during a software installation, one can track file creations, modifications and deletions.
This monitoring quickly shows that the installer creates a .bat file. This is a MySQL database configuration script, and contains the queries observed in the CodeSection.txt file seen in the previous method.

![Process monitor](/assets/images/post-20240413/2_procmon.png)


This file is supposed to be deleted immediately after execution, but it is easy to copy it before deletion. In the event that the file is deleted too quickly for a human to copy it, one can create a script that would monitor the creation of files in the affected directory and perform the copy automatically.


![Script](/assets/images/post-20240413/2_script_save.png)

(Call me lazy, but this just works)


We can therefore open and inspect the file, note the creation and modification requests from MySQL users, and extract the passwords.

![Bat script](/assets/images/post-20240413/2_script_bat.png)

- [Sysinternals procmon](https://learn.microsoft.com/fr-fr/sysinternals/downloads/procmon)


### Option 3 : Decompile the .class files

Once the software is installed, you can see in the installation folder numerous .jar files specific to Java applications. As a reminder, Java is compiled into bytecode, which is then interpreted by a virtual machine.

This is interesting to us because the bytecode can be decompiled to get Java code very similar to the original code, making it pretty easy to read and understand.

Among the JAR files, we can try to identify those that may be interesting for us, with a name containing the words "connection", "admin", or "mysql" for example.

If no particular JAR file stands out, you can always decompile the whole thing and look for SQL queries in the code, then rewind the code until you find code making the connection to MySQL, and finally find dabase creds.

In my case, I quickly found a class which contains several constants used in various parts of the software, including much wanted MySQL access.

![Decompile JAVA](/assets/images/post-20240413/3_jad_creds.png)

- [Java Decompiler](http://java-decompiler.github.io/)


### Option 4 : Digging in the process dump

For this last method, you must obtain a dump of the process of the application in question.

As it is a Java application, a specific tool exists in the SDK: jmap.
It is also possible to create a "dump file" from the Windows task manager.

In our case, both solutions are valid. Dumping a process means we obtain a file which represents the state of the application at this time. 

A process dump contains many infos about a running application, its code and memory.
Maybe we will find something interesting inside such dump ?

![Dump process memory](/assets/images/post-20240413/4_dump.png)

Let's open the file with a hex editor (I like HxD)
By searching for "mysql", "password" or the mysql username if found before, you can easily find the password several times

![Dump process memory](/assets/images/post-20240413/4_dump3.png)


- [Sysinternals ProcessExplorer](https://learn.microsoft.com/en-us/sysinternals/downloads/process-explorer)
- [HxD](https://mh-nexus.de/en/hxd/)


### Some additionnal thoughts

As a last part, I'll add a note on network capture and hash cracking.


##### Network capture with wireshark

MySQL can communicate over network plaintext or encrypted (TLS).

In my case, MySQL paquets were indeed encrypted. If we open again our .bat setup file, we see that this script generates RSA keys with OpenSSL in order to encrypt the exchanges between the MySQL server and the application. 

![Network capture encrypted](/assets/images/post-20240413/5_network_encrypted.png)

Obviously, the key files (PEM extension) must be somewhere in software's directory so it can initiate connexion at each startup.
We just need to find those PEM files and add them in Wireshark, so TLS packets get decrypted back to MySQL packets.

MySQL auth protocol being secure enough, we won't intercept mysql users' creds that way.
But we can now see SQL queries in plaintext, meaning we could maybe grab sensitive data, such as software-level credentials, columns and tables names, maybe potentials SQLi vectors ?

![Network capture plaintext](/assets/images/post-20240413/5_network_plaintext.png)


- [Decrypt TLS using Wireshark](https://my.f5.com/manage/s/article/K19310681)


##### Set up a rogue MySQL server

Metasploit auxiliaries tools include a rogue MySQL server, so if we can force our software to connect to our MySQL server, we will receive a challenge/response auth.

In case we can't configure a software to connect our rogue server, we still can set up a socat to redirect traffic on port 3306 to our server.

We can set our server to write a John format file and crack it right away.

![Rogue MySQL](/assets/images/post-20240413/5_rogue.png)

- [Metasploit's MySQL server](https://www.infosecmatter.com/metasploit-module-library/?mm=auxiliary/server/capture/mysql)


##### Crack'em all

So we could grab MySQL hashes with at least two methods : from internal mysql.user table, and intercepting challenge/response with a rogue server.
So now, how do we retrieve 

**Hash from mysql.user**
```# with hashcat (WITHOUT leading *)
hashcat -a 0 -m 300 hashes.txt wordlist.txt

# with john (WITH leading *)
john -w=wordlist.txt hash.txt --format=mysql-sha1
```

**Hash from rogue server**
```# with hashshcat
hashcat -a 0 -m 11200 hashes.txt wordlist.txt --username

# with john
john -w=wordlist.txt hash_mysqlna
```
![Rogue MySQL](/assets/images/post-20240413/5_crack_hash.png)

- [Hashcat hash formats](https://hashcat.net/wiki/doku.php?id=example_hashes)
- [john hash formats](https://pentestmonkey.net/cheat-sheet/john-the-ripper-hash-formats)