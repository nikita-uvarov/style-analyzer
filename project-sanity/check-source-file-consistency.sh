#!/bin/bash

# Checks that a source file corresponds to project style:
# - checks indentation, usage of spaces instead of tabs etc. via astyle
# - manually checks all lines are less than fixed width long

readonly MAX_LINE_LENGTH=130
readonly TEMPORARY_FILE="/tmp/reference-indent"
readonly ASTYLE_OPTIONS="--style=allman"`                 # Allman braces style
                        `" --indent=spaces=4"`            # Do not use tabs at all
                        `" --convert-tabs"`               # Get rid of them!
                        `" --max-instatement-indent=120"` # Allow to align function arguments even if the name is pretty long
                        `" --keep-one-line-statements"`   # Keep one-liners (e.g. case type: return "type";)
                        `" --indent-switches"`            # When switches are needed (small case blocks), indentation looks fine
                        `""

WERE_ERRORS=0

function markError()
{
    WERE_ERRORS=1
}

# Does not recognize tabs as multiple characters - get rid of tabs earlier
function checkMaxLineLength()
{
    local LINE_NUMBER=1
    while read -r line; do
        if [ ${#line} -gt $MAX_LINE_LENGTH ]; then
            echo "Max line length $MAX_LINE_LENGTH exceeded: length ${#line} on line $LINE_NUMBER:"
            echo "$line"
            markError
        fi;
        LINE_NUMBER=$(($LINE_NUMBER+1))
    done < "$1"
}

function checkFileIndentation()
{
    astyle $ASTYLE_OPTIONS <"$1" >"$TEMPORARY_FILE"

    if ! diff -u "$1" "$TEMPORARY_FILE"; then
        echo "The indentation of file differs from astyle sample."
        markError
    fi
}

checkMaxLineLength "$1"
checkFileIndentation "$1"

exit $WERE_ERRORS
