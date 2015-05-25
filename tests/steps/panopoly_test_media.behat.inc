<?php
/**
 * @file
 * Provide Behat step-definitions for the Media module.
 *
 * @todo Also, add a cleanup function to delete files uploaded.
 */

use Drupal\DrupalExtension\Context\DrupalSubContextInterface;
use Drupal\DrupalDriverManager;
use Drupal\DrupalExtension\Context\RawDrupalContext;

class MediaSubContext extends RawDrupalContext implements DrupalSubContextInterface {

  private $drupal;

  /**
   * Initializes context.
   */
  public function __construct(DrupalDriverManager $drupal) {
    $this->drupal = $drupal;
  }

  public static function getAlias() {
    return 'fieldable_panels_panes';
  }

  /**
   * @When /^I click the fake "([^"]*)" button$/
   */
  public function iClickTheFakeButton($text) {
    // Media style selector "buttons" are A tags with no href, so not findable
    // by normal steps.
    $driver = $this->getSession()->getDriver();
    $buttons = $driver->find("//a[text()='$text']");
    $buttons[0]->click();
  }
}
