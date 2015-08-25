<?php
/**
 * @file
 * Provide Behat step-definitions for keyboard focus actions.
 *
 */

use Drupal\DrupalExtension\Context\DrupalSubContextInterface;
use Drupal\DrupalDriverManager;
use Drupal\DrupalExtension\Context\RawDrupalContext;

class KeyboardSubContext extends RawDrupalContext implements DrupalSubContextInterface {

  /**
   * Contains the DrupalDriverManager.
   *
   * @var \Drupal\DrupalDriverManager
   */
  private $drupal;

  /**
   * Initializes context.
   */
  public function __construct(DrupalDriverManager $drupal) {
    $this->drupal = $drupal;
  }

  protected function getFocusedElementId() {
    $focusedElementId = $this->getSession()->evaluateScript("return jQuery(document.activeElement).attr('id')");
    return $focusedElementId;
  }

  /**
   * @When I focus on the element with the ID :id
   */
  public function iFocusOnTheElementWithTheId($id) {
    // Find the element by ID and focus it.
    $element = $this->getSession()->getPage()->findById($id);
    if (!$element) {
      throw new \Exception(sprintf('The element with the ID "%s" was not found on the page %s', $id, $this->getSession()->getCurrentUrl()));
    }
    $element->focus();

    // Check that the element was actually focused - some elements are not focusable.
    $focusedElement = $this->getFocusedElementId();
    if ($id != $focusedElement) {
      throw new \Exception(sprintf('The element with the ID "%s" could not be focused', $id));
    }

  }

  /**
   * @Then the element with the ID :id should be focused
   */
  public function elementWithIdShouldBeFocused($id) {
    // Find the element by ID.
    $element = $this->getSession()->getPage()->findById($id);
    if (!$element) {
      throw new \Exception(sprintf('The element with the ID "%s" was not found on the page %s', $id, $this->getSession()->getCurrentUrl()));
    }
    // Get the ID of the current active element.
    $focusedElement = $this->getFocusedElementId();
    if ($id != $focusedElement) {
      throw new \Exception(sprintf('The element with the ID "%s" was not focused, "%s" was', $id, $focusedElementId));
    }
  }
}
