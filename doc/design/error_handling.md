#  Error handling strategy

API calls must be safe, even if rendering failed. This means that any call to any api command with any parameter should not result in a program halt. Only current status should be affected by errors.

## Per object status.

Internally, statuses are first checked for each function calls. If objects are in error, api calls will safely return without performing any action.
The Status field in each vkvg structure being the first member, even a memory allocation failure may return a safe pointer holding only a pointer to VKVG_STATUS_NO_MEMORY static value.

Client applications may check current statuses at anytime, even on a null pointer, to ensure normal operations.



