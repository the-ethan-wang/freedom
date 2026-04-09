# Freedom calculator

> The sha256sum of every message (as written in plaintext) must begin with the same 4 bytes as the message itself.  

I interpret this as: The bytes of the sha256sum of a message must begin with the same 4 bytes as the bytes of the message encoded with ascii  

Thus, you can find the hex representation of the sha256 sum and the hex representation of the message and compare the first 4 bytes of these.  

This means that the message must be at least 4 characters, since 1 character in ascii is 1 byte, and the sha256 is definitely longer than 4 bytes.  

## Currently found:  
- ,bsO?

## Checked:
- 4-5 character messages using alphanumeric + space + new line + .,?!  

## TODO:

- optimise and write in cpp
- include all discord-allowed characters

## Optimisations

- Instead of getting the hex of the entire string then looking at the first 4 bytes, you only need the hex of the first 8 characters
