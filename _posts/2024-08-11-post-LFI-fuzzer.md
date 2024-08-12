---
title: "LFI fuzzer"
categories:
  - Pentest
tags:
  - Python, Webapp, File Inclusion
---


### Why another fuzzer ?

Some time ago I completed [TryHackMe's File Inclusion, Path Traversal room](https://tryhackme.com/r/room/filepathtraversal). This room is pretty handy to try and work LFI/RFI on a PHP webapp.

Until then, I was pretty used to detect and exploit LFI by hand, or as often seen, with a general purpose fuzzer such as FFUF.

Nothing wrong about FFUF, but since it is a general purpose fuzzer, it will only send "dumb" requests based on a wordlist, so the effectiveness of your pentest will mostly rely on how good your wordlist is.

One can find many different wordlists [here](https://github.com/carlospolop/Auto_Wordlists/blob/main/wordlists/file_inclusion_linux.txt), [here](https://github.com/emadshanab/LFI-Payload-List/blob/master/LFI%20payloads.txt) or [there](https://github.com/DragonJAR/Security-Wordlist/blob/main/LFI-WordList-Linux)

Some wordlists only contains absolute paths to useful files, but then it is a pain to adapt the payload : how to find the good path traversal, encoding and other evasion techniques ?

Other wordlists include path traversal and encodings, but these may not be consistent and it's hard to be sure every variations for every possible file is present. Also, this kind of wordlist is easily bloated from so many variations and would be hard to maintain.

Acknowledging these issues, the idea is to have a specific, simple tool to LFI/RFI fuzzing, with enough options to be adequate to most needs.

Enough words of introduction, let's highlight some key features. A complete list of arguments is available on the [LFI fuzzer repo](https://github.com/20100dbg/Python-Stuff/tree/master/LFI_fuzzer)


### Key features

###### -p / --path-prefix
This add a prefix before your payload.
For example : `-p xxx/`

Will result in something like `http://victim.com/?page=xxx/../../../etc/passwd`

This allows to bypass some basic checks trying to determine if a requested page is in a specific directory : `substr($page, 0, 8) == 'include/'`

Moreover, this option allows you to add a PHP filter : `-p php://filter/convert.base64-encode/resource=`

Will result in : `http://victim.com/?page=php://filter/convert.base64-encode/resource=../../../etc/passwd`


###### -m / --traversal-method
This will replace classic path traversal `../`.
For example : `-m 2`
Will result in `http://victim.com/?page=....//....//etc/passwd`

This will bypass simple protections against path traversal such as `$page = str_replace($page, '../', '')`


###### -s / --stress
The stress option will perform every variation for every parameter on each item of a given wordlist. 
This is a useful bruteforce option if you are in a hurry, and if you don't care about being noisy.


#### Sucess/error detection

Ignore if a result length is < 1300
```
--min-length 1300
```
Looking for a specific string in files
/etc/passwd should contains `root:x:0:0`
```
--success-string "root:x:0:0"
```

c:\windows\win.ini should contains "for 16-bit app support"
```
--success-string "for 16-bit app support"
```

Possible error message
```
--error-string "page not found"
```


That's all folks ! I hope you'll find this tool as useful as I do !

