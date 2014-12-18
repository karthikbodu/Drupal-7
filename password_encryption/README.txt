
INTRODUCTION
------------
This module helps you to generate the encrypted password for
a provided password and you can update the "pass" field for
the user by running the mysql command.

INSTALLATION
------------
Install as you would normally install a contributed drupal module. See:
https://drupal.org/documentation/install/modules-themes/modules-7
for further information.

***** This module is not recommended for production sites *****

CONFIGURATION
-------------
Follow the below steps to update the user password
with the encrypted password

1) Generate a encrypted password

 * Go to (admin/config/people/password/encrypt)
   or (admin -> Configuration -> People » Password Encryption)
                 OR
 * Go to (user/encrypt-password) or click on " Password Encryption "
   from the user login block.
   
2) Copy the encrypted text to the clipboard and use this
 to set as a value to the "pass" field for a user.

3) Run a mysql command like
 "UPDATE users SET 
 pass='$S$Ddd06jN2Lu7AQWPwmYCfT17kmokuYfuFm7TqbvR955uriqtUXIEX' WHERE uid=1;"
