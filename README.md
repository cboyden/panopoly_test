Behat tests
===========

Quick setup
-----------

 
 1. Install Composer

    php -r "eval('?>'.file_get_contents('https://getcomposer.org/installer'));"
 
 2. Install Behat and dependencies via Composer

    php composer.phar install

 3. Copy behat.template.yml to behat.yml and modify

    mv behat.template.yml behat.yml
 
 4. Enable the panopoly_test module

    drush en panopoly_test

 5. Run Behat and examine test results!
 
    bin/behat

More information
----------------

For detailed instructions, see:

  https://drupal.org/node/2271009

