# Freedom calculator

AI NOTICE: I don't know cpp or optimisation so the cpp parts of this were written by ai

> The sha256sum of every message (as written in plaintext) must begin with the same 4 bytes as the message itself.  

I interpret this as: The bytes of the sha256sum of a message must begin with the same 4 bytes as the bytes of the message encoded with ascii  

Thus, you can find the hex representation of the sha256 sum and the hex representation of the message and compare the first 4 bytes of these.  

This means that the message must be at least 4 characters, since 1 character in ascii is 1 byte, and the sha256 is definitely longer than 4 bytes.  

To optimise the search, I left my computer running for 1.5 hours at 100% cpu usage and couldn't use it because yk this is using all of my cpu.  

Found 0 4 character messages  
Found 1 5 character message  
Found 28 6 character messages  

I will add a thingy where you input a string and it finds the characters to add to the end/start to make it valid, and minimising these(maybe)

To scan all of the 5 character messages:  
- `main.py` took ~52 minutes 
- `main.cpp` took ~13 minutes
- `multithread.cpp` took ~8 minutes
- `multi5.cpp` took ~6 minutes
- `multimulti5.cpp` took ~1 minute

To scan all of the 6 character messages:
- `multi6.cpp` projected to take ~6 hours
- `multimulti6.cpp` took ~1.5 hours

## Currently found:  
- ,bsO?

- c4oMUr
- hLe5eZ
- k5KoL5
- lgCMSy
- lGjLqq
- p,2goD
- pNey2M
- qCun3S
- sCZsyj
- vAj!6e
- x96LXO
- z6EXiK
- C9TH(\n)2
- D(\n)XI4w
- GCtEN1
- HKmCZA
- K9Kjtp
- Lc2P6l
- PjF,UB
- Rp9!gc
- UFVuSt
- U2MQgA
- W!O.6Z
- WT6f3M
- 6ciEcp
- 6wwAgN
- 9Jti?4
- M3!5R

## Checked:
- 4-6 character messages using alphanumeric + space + new line + .,?!  

## TODO:

- optimise and write in cpp
- include all discord-allowed characters
- program to find valid message that starts with given string

## Optimisations

- Instead of getting the hex of the entire string then looking at the first 4 bytes, you only need the hex of the first 8 characters
