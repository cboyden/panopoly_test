<?php
/**
 * @file
 * Provide Behat step-definitions for Panels.
 *
 * @todo This should become panels.behat.inc in the Panels module!
 */

use Drupal\DrupalExtension\Context\DrupalSubContextInterface;
use Drupal\DrupalDriverManager;
use Drupal\Component\Utility\Random;
use Drupal\DrupalExtension\Context\RawDrupalContext;

class PanelsSubContext extends RawDrupalContext implements DrupalSubContextInterface {

  private $drupal;

  /**
   * Initializes context.
   */
  public function __construct(DrupalDriverManager $drupal) {
    $this->drupal = $drupal;
  }

  public static function getAlias() {
    return 'panels';
  }

  /**
   * @When /^(?:|I )wait for the Panels IPE to activate$/
   *
   * Wait until the Panels IPE is activated.
   */
  public function waitForIPEtoActivate() {
    $this->getSession()->wait(5000, 'jQuery(".panels-ipe-editing").length > 0');
  }

  /**
   * @When /^(?:|I )wait for the Panels IPE to deactivate$/
   *
   * Wait until the Panels IPE is deactivated.
   */
  public function waitForIPEtoDeactivate() {
    $this->getSession()->wait(5000, 'jQuery(".panels-ipe-editing").length === 0');
  }

  /**
   * @When /^(?:|I )customize this page with the Panels IPE$/
   *
   * Enable the Panels IPE if it's available on the current page.
   */
  public function customizeThisPageIPE() {
    $this->getSession()->getPage()->clickLink('Customize this page');
    $this->waitForIPEtoActivate();
  }

  /**
   * @When /^(?:|I )change layout with the Panels IPE$/
   *
   * Open 'Change layout' dialog if it's available on the current page.
   */
  public function changeLayoutIPE() {
    $this->getSession()->getPage()->clickLink('Change layout');
    $this->getSession()->wait(5000, 'jQuery(".panels-choose-layout").length === 0');
  }

}
