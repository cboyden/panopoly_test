Feature: Test keyboard accessibility
  In order to test keyboard accessibility
  As a site administrator
  I need to be able to check focus

  @api @javascript @keyboard @chrome
  Scenario: Check focus: 
    Given I am at "/demo"
    When I focus on the element with the ID "edit-keys" 
    Then the element with the ID "edit-keys" should be focused
  