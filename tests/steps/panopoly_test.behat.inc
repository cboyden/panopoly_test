<?php
/**
 * @file
 * Provide Behat step-definitions for generic Panopoly tests.
 *
 */
//require_once(dirname(__FILE__).'/../features/bootstrap/PanopolyContext.php');
use Drupal\DrupalExtension\Context\DrupalSubContextInterface;
use Drupal\DrupalDriverManager;
use Drupal\Component\Utility\Random;
use Drupal\DrupalExtension\Context\RawDrupalContext;

class TestSubContext extends RawDrupalContext implements DrupalSubContextInterface {

  /**
   * Holds a list of temporary users created by our steps.
   *
   * @var array
   */
  protected $users = array();

  private $drupal;

  /**
   * Initializes context.
   */
  public function __construct(DrupalDriverManager $drupal) {
    $this->drupal = $drupal;
  }

  public static function getAlias() {
    return 'test';
  }

  protected $private_path_overridden = FALSE;

  public function getRegion($region) {
    $session = $this->getSession();
    $regionObj = $session->getPage()->find('region', $region);
    if (!$regionObj) {
      throw new \Exception(sprintf('No region "%s" found on the page %s.', $region, $session->getCurrentUrl()));
    }
    return $regionObj;
  }

  /**
   * @When /^I switch to the frame "([^"]*)"$/
   */
  public function iSwitchToTheFrame($frame) {
    $this->getSession()->switchToIFrame($frame);
  }

  /**
   * @When /^I switch out of all frames$/
   */
  public function iSwitchOutOfAllFrames() {
    $this->getSession()->switchToIFrame();
  }

  /**
   * @Then I( should) see :text in the :tag element in the :region( region)
   */
  public function assertRegionElementText($text, $tag, $region) {
    $regionObj = $this->getRegion($region);
    $elements = $regionObj->findAll('css', $tag);
    if (empty($elements)) {
      throw new \Exception(sprintf('The element "%s" was not found in the "%s" region on the page %s', $tag, $region, $this->getSession()->getCurrentUrl()));
    }

    $found = FALSE;
    foreach ($elements as $element) {
      if ($element->getText() == $text) {
        $found = TRUE;
        break;
      }
    }
    if (!$found) {
      throw new \Exception(sprintf('The text "%s" was not found in the "%s" element in the "%s" region on the page %s', $text, $tag, $region, $this->getSession()->getCurrentUrl()));
    }
  }

  /**
   * Checks, that the region contains text matching specified pattern.
   *
   * @Then /^(?:|I )should see text matching "(?P<pattern>(?:[^"]|\\")*)" in the "(?P<region>[^"]*)"(?:| region)$/
   */
  public function assertRegionMatchesText($pattern, $region)
  {
    $regionObj = $this->getRegion($region);

    // Find the text within the region
    $regionText = $regionObj->getText();
    if (!preg_match($pattern, $regionText)) {
      throw new \Exception(sprintf("No text matching '%s' was found in the region '%s' on the page %s", $pattern, $region, $this->getSession()->getCurrentUrl()));
    }
  }

  /**
   * Checks, that the region does not contain text matching specified pattern.
   *
   * @Then /^(?:|I )should not see text matching "(?P<pattern>(?:[^"]|\\")*)" in the "(?P<region>[^"]*)"(?:| region)$/
   */
  public function assertNotRegionMatchesText($pattern, $region)
  {
    $regionObj = $this->getRegion($region);

    // Find the text within the region
    $regionText = $regionObj->getText();
    if (preg_match($pattern, $regionText)) {
      throw new \Exception(sprintf("Text matching '%s' was found in the region '%s' on the page %s", $pattern, $region, $this->getSession()->getCurrentUrl()));
    }
  }

  /**
   * @Then /^I should see an image in the "([^"]*)" region$/
   *
   * Attempts to determine if the image is broken or not.
   */
  public function assertValidImageRegion($region) {
    $regionObj = $this->getRegion($region);
    $elements = $regionObj->findAll('css', 'img');
    if (empty($elements)) {
      throw new \Exception(sprintf('No image was not found in the "%s" region on the page %s', $region, $this->getSession()->getCurrentUrl()));
    }

    if ($src = $elements[0]->getAttribute('src')) {
      $params = array('http' => array('method' => 'HEAD'));
      $context = stream_context_create($params);
      $fp = @fopen($src, 'rb', FALSE, $context);
      if (!$fp) {
        throw new \Exception(sprintf('Unable to download <img src="%s"> in the "%s" region on the page %s', $src, $region, $this->getSession()->getCurrentUrl()));
      }

      $meta = stream_get_meta_data($fp);
      fclose($fp);
      if ($meta === FALSE) {
        throw new \Exception(sprintf('Error reading from <img src="%s"> in the "%s" region on the page %s', $src, $region, $this->getSession()->getCurrentUrl()));
      }

      $wrapper_data = $meta['wrapper_data'];
      $found = FALSE;
      if (is_array($wrapper_data)) {
        foreach ($wrapper_data as $header) {
          if (substr(strtolower($header), 0, 19) == 'content-type: image') {
            $found = TRUE;
          }
        }
      }

      if (!$found) {
        throw new \Exception(sprintf('Not a valid image <img src="%s"> in the "%s" region on the page %s', $src, $region, $this->getSession()->getCurrentUrl()));
      }
    }
    else {
      throw new \Exception(sprintf('No image had no src="..." attribute in the "%s" region on the page %s', $region, $this->getSession()->getCurrentUrl()));
    }
  }


  /**
   * @Then /^I should see the image alt "(?P<alt>(?:[^"]|\\")*)" in the "(?P<region>[^"]*)"(?:| region)$/
   */
  public function assertAltRegion($alt, $region) {
    $regionObj = $this->getRegion($region);
    $element = $regionObj->find('css', 'img');
    $tmp = $element->getAttribute('alt');
    if ($alt == $tmp) {
      $result = $alt;
    }
    if (empty($result)) {
      throw new \Exception(sprintf('No alt text matching "%s" in the "%s" region on the page %s', $alt, $region, $this->getSession()->getCurrentUrl()));
    }
  }

  /**
   * @BeforeScenario @javascript
   *
   * Set a variable so AfterStep knows this is a javascript scenario.
   */
  public function setJavascript() {
    $this->javascript = true;
  }

  /**
   * @AfterScenario @javascript
   *
   * Unset the variable so other scenarios don't run the javascript AfterStep.
   */
  public function unsetJavascript() {
      $this->javascript = false;
    }

  /**
   * @BeforeScenario @api&&@drupal_private_files
   *
   * Configure a private files path if one isn't already configured.
   */
  public function configurePrivateFiles($event) {
    $file_private_path = variable_get('file_private_path', '');
    if (empty($file_private_path)) {
      $file_public_path = variable_get('file_public_path', conf_path() . '/files');
      if (empty($file_public_path)) {
        throw new \Exception('Files must be configured for @drupal_private_files tests to work!');
      }

      // Set flag for later cleanup.
      $this->private_path_overridden = TRUE;

      // Create and setup the private path.
      $file_private_path = $file_public_path . '/' . 'private';
      variable_set('file_private_path', $file_private_path);
    }
  }

  /**
   * @AfterScenario @api&&@drupal_private_files
   *
   * Clean up our temporary private files path.
   */
  public function cleanupPrivateFiles($event) {
    if ($this->private_path_overridden) {
      variable_del('file_private_path');
    }
  }

  /**
   * @AfterScenario
   *
   * Clean up users that were created during testing
   */
  public function removeUsers() {
    if(!empty($this->users)){
      foreach ($this->users as $user) {
        $account = user_load_by_name($user->name);
        user_delete($account->uid);
      }
      $this->users = array();
    }
  }

  /**
   * @AfterStep
   *
   * After every step in a @javascript scenario, we want to wait for AJAX
   * loading to finish.
   */
  public function afterStep($event) {
    if (isset($this->javascript) && $this->javascript) {
      $text = $event->getStep()->getText();
      if (preg_match('/(follow|press|click|submit|viewing|visit|reload)/i', $text)) {
        $this->iWaitForAjax();
      }
    }
  }

  /**
   * @Given /^Panopoly magic live previews are disabled$/
   *
   * Disable live previews via Panopoly Magic.
   */
  public function disablePanopolyMagicLivePreview() {
    //$this->getDriver('drush')->vset('panopoly_magic_live_preview 0 --yes');
    variable_set('panopoly_magic_live_preview', 0);
  }

  /**
   * @Given /^Panopoly magic live previews are automatic$/
   *
   * Enable live previews via Panopoly Magic.
   */
  public function enableAutomaticPanopolyMagicLivePreview() {
    //$this->getDriver('drush')->vset('panopoly_magic_live_preview 1 --yes');
    variable_set('panopoly_magic_live_preview', 1);
  }

  /**
   * @Given /^Panopoly magic live previews are manual$/
   *
   * Enable live previews via Panopoly Magic.
   */
  public function enableManualPanopolyMagicLivePreview() {
    //$this->getDriver('drush')->vset('panopoly_magic_live_preview 2 --yes');
    variable_set('panopoly_magic_live_preview', 2);
  }

  /**
   * @Given /^Panopoly admin "Use Advanced Panel Plugins" is disabled$/
   *
   * Disable the "Use Advanced Panel Panes" option.
   */
  public function disablePanopolyAdminAdvanacedPanelPlugins() {
    variable_set('panopoly_admin_advanced_plugins', FALSE);
  }

  /**
   * @Given /^Panopoly admin "Use Advanced Panel Plugins" is enabled$/
   *
   * Enable the "Use Advanced Panel Panes" option.
   */
  public function enablePanopolyAdminAdvanacedPanelPlugins() {
    variable_set('panopoly_admin_advanced_plugins', TRUE);
  }

  /**
   * @When /^(?:|I )wait(?:| for) (\d+) seconds?$/
   *
   * Wait for the given number of seconds. ONLY USE FOR DEBUGGING!
   */
  public function iWaitForSeconds($arg1) {
    sleep($arg1);
  }

/**
   * Wait for AJAX to finish.
   *
   * @Given I wait for AJAX
   */
  public function iWaitForAjax() {
    $this->getSession()->wait(5000, '(typeof(jQuery)=="undefined" || (0 === jQuery.active && 0 === jQuery(\':animated\').length))');
  }

 /**
   * @When /^(?:|I )wait for live preview to finish$/
   *
   * Wait until the live preview to finish.
   */
  public function waitForLivePreview() {
    $this->getSession()->wait(5000, '(typeof(jQuery)=="undefined" || (0 === jQuery.active && 0 === jQuery(\':animated\').length)) && jQuery("#panopoly-form-widget-preview").length > 0 && !jQuery("#panopoly-form-widget-preview").hasClass("panopoly-magic-loading")');
  }

  /**
   * @Given /^I log in with the One Time Login Url$/
   */
  public function iLogInWithTheOneTimeLoginUrl() {
    if ($this->loggedIn()) {
      $this->logOut();
    }

    $random = new Random;

    // Create user (and project)
    $user = (object) array(
      'name' => $random->name(8),
      'pass' => $random->name(16),
      'role' => 'authenticated user',
    );
    $user->mail = "{$user->name}@example.com";

    // Create a new user.
    $this->getDriver()->userCreate($user);

    $this->users[$user->name] = $this->user = $user;

    $base_url = rtrim($this->locatePath('/'), '/');
    $login_link = $this->getDriver('drush')->drush('uli', array(
      "'$user->name'",
      '--browser=0',
      "--uri=${base_url}",
    ));
    // Trim EOL characters. Required or else visiting the link won't work.
    $login_link = trim($login_link);
    $login_link = str_replace("/login", '', $login_link);
    $this->getSession()->visit($this->locatePath($login_link));
    return TRUE;
  }

  /**
   * @Given /^I am viewing a landing page$/
   */
  public function iAmViewingALandingPage() {
    $random = new Random();
    $name = $random->name(8);
    $this->createNode('panopoly_test_landing_page', $name);
  }

  /**
   * @Then /^the "([^"]*)" radio button should be set to "([^"]*)"$/
   *
   * @link: https://www.drupal.org/node/1891584 @endlink
   */
  public function theRadioButtonShouldBeSetTo($identifier, $option) {
    $page = $this->getSession()->getPage();
    $div = $page->find('xpath', "//div[contains(., '$identifier') and @class[contains(.,'form-type-radio')]]");
    if ($div) {
      $radios = $div->find('xpath', "//input[@type='radio']");
      if ($radios) {
        $checkedRadio = $div->find('xpath', "//input[@type='radio' and @checked='checked']/following-sibling::label[contains(text(), '$option')] ");
        if (!$checkedRadio) {
          throw new \Exception(sprintf('We found the radio buttons for "%s", but "%s" was not selected.', $identifier, $option));
        }
      } elseif (!$radios) {
        throw new \Exception(sprintf('We found "%s", but it did not contain any radio buttons".', $identifier));
      }
    } elseif (!$div) {
      throw new \Exception(sprintf('We couldn\'nt find "%s" on the page', $identifier));
    } else {
      throw new \Exception('General exception from '.__FUNCTION__);
    }
  }

  /**
   * @Then /^I should see the radio button "(?P<label>[^"]*)" with the id "(?P<id>[^"]*)"$/
   * @Then /^I should see the radio button "(?P<label>[^"]*)"$/
   */
  public function assertSeeRadioById($label, $id = FALSE) {
    $element = $this->getSession()->getPage();
    $radiobutton = $id ? $element->findById($id) : $element->find('named', array('radio', $this->getSession()->getSelectorsHandler()->xpathLiteral($label)));
    if ($radiobutton === NULL) {
      throw new \Exception(sprintf('The radio button with "%s" was not found on the page %s', $id ? $id : $label, $this->getSession()->getCurrentUrl()));
    }
    if ($id) {
      $value = $radiobutton->getAttribute('value');
      $labelonpage = $radiobutton->getParent()->getText();
      if ($label != $labelonpage) {
        throw new \Exception(sprintf("Button with id '%s' has label '%s' instead of '%s' on the page %s", $id, $labelonpage, $label, $this->getSession()->getCurrentUrl()));
      }
    }
  }

  /**
   * @Then /^I should not see the radio button "(?P<label>[^"]*)" with the id "(?P<id>[^"]*)"$/
   * @Then /^I should not see the radio button "(?P<label>[^"]*)"$/
   */
  public function assertNotSeeRadioById($label, $id = FALSE) {
    $element = $this->getSession()->getPage();
    $radiobutton = $id ? $element->findById($id) : $element->find('named', array('radio', $this->getSession()->getSelectorsHandler()->xpathLiteral($label)));
    if ($radiobutton !== NULL) {
      throw new \Exception(sprintf('The radio button with "%s" was found on the page %s', $id ? $id : $label, $this->getSession()->getCurrentUrl()));
    }
  }

  /**
   * @Then /^the "([^"]*)" select should be set to "([^"]*)"$/
   */
  public function theSelectShouldBeSetTo($identifier, $value) {
    $select = $this->getSession()->getPage()->findField($identifier);
    if (empty($select)) {
      throw new \Exception(sprintf('We couldn\'nt find "%s" on the page', $identifier));
    }
    $options = $select->findAll('xpath', '//option[@selected="selected"]');
    if (empty($select)) {
      throw new \Exception(sprintf('The select "%s" doesn\'t have any options selected', $identifier));
    }

    $found = FALSE;
    foreach ($options as $option) {
      if ($option->getText() === $value) {
        $found = TRUE;
        break;
      }
    }
    if (!$found) {
      throw new \Exception(sprintf('The select "%s" doesn\'t have the option "%s" selected', $identifier, $value));
    }
  }

  /**
   * @Given /^the dblog is empty$/
   */
  public function clearDblog() {
    db_delete('watchdog')->execute();
  }
  
  /**
   * @When /^I select the first autocomplete option for "([^"]*)" on the "([^"]*)" field$/
   */
  public function iSelectFirstAutocomplete($prefix, $identifier) {
    $session = $this->getSession();
    $page = $session->getPage();
    $element = $page->findField($identifier);
    if (empty($element)) {
      throw new \Exception(sprintf('We couldn\'t find "%s" on the page', $identifier));
    }
    $page->fillField($identifier, $prefix);

    $xpath = $element->getXpath();
    $driver = $session->getDriver();

    // autocomplete.js uses key down/up events directly.

    // Press the backspace key.
    $driver->keyDown($xpath, 8);
    $driver->keyUp($xpath, 8);

    // Retype the last character.
    $chars = str_split($prefix);
    $last_char = array_pop($chars);
    $driver->keyDown($xpath, $last_char);
    $driver->keyUp($xpath, $last_char);

    // Wait for AJAX to finish.
    $this->iWaitForAJAX();

    // And make sure the autocomplete is showing.
    $this->getSession()->wait(5000, 'jQuery("#autocomplete").show().length > 0');

    // And wait for 1 second just to be sure.
    sleep(1);

    // Press the down arrow to select the first option.
    $driver->keyDown($xpath, 40);
    $driver->keyUp($xpath, 40);

    // Press the Enter key to confirm selection, copying the value into the field.
    $driver->keyDown($xpath, 13);
    $driver->keyUp($xpath, 13);

    // Wait for AJAX to finish.
    $this->iWaitForAJAX();
  }
}
