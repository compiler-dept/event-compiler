# Scopes
Scopes are nested namespaces which are used to link references within the ast to
their corresponding declarations. Therefore within each scope names must be unique.

## Global scope

- predicate_definition
- function_definition
- event_declaration
- (rule_declaration : is not needed)

## Local scopes

### event_declaration

- member (IDENTIFIER)

### rule_declaration

- event (TYPE)

### predicate_definition

- parameter (IDENTIFIER)

### function_definition

- parameter (IDENTIFIER)
