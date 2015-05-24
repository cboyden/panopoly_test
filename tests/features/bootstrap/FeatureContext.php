<?php

require_once 'PanopolyContext.php';

/**
 * Features context.
 */

/**
 * Define application features from the specific context.
 */
class FeatureContext extends PanopolyContext {
  /**
   * Initializes context.
   * Every scenario gets its own context object.
   *
   * @param array $parameters
   *   Context parameters (set them in behat.yml)
   */
  public function __construct() {
    // Initialize your context here
  }

//
// Place your definition and hook methods here:
//
//  /**
//   * @Given I have done something with :stuff
//   */
//  public function iHaveDoneSomethingWith($stuff) {
//    doSomethingWith($stuff);
//  }
//
}