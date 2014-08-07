#
# Sed script to parse CPP macros and generate output usable by make
#
# It is expected that this script is fed the output of 'gpp -dM'
# which preprocesses the common.h header files and outputs the final
# list of CPP macros (and whitespace is sanitized)
#

# Only process values prefixed with #define CONFIG_
/^#define CONFIG_[A-Za-z0-9_]\+/ {
	# Strip the #define prefix
	s/#define *//;
	# Change to form CONFIG_*=VALUE
	s/ \+/=/;
	# Drop trailing spaces
	s/ *$//;
	# drop quotes around string values
	s/="\(.*\)"$/=\1/;
	# Concatenate string values
	s/" *"//g;
	# Wrap non-numeral values with quotes
	s/=\(.*\?[^0-9].*\)$/=\"\1\"/;
	# Change '1' and empty values to "y" (not perfect, but
	# supports conditional compilation in the makefiles
	s/=$/=y/;
	s/=1$/=y/;
	# print the line
	p
}

# Also, add the definition for "CFG_SDRAM_BASE".
# This is for the SH boards, where the address of SDRAM can vary
# depending on the exact board, and if it is in 29 or 32-bit mode.
# Used by examples/Makefile
/^#define CFG_SDRAM_BASE/ {
	# Strip the #define prefix
	s/#define *//;
	# Change to form CONFIG_*=VALUE
	s/ \+/=/;
	# Drop trailing spaces
	s/ *$//;
	# drop quotes around string values
	s/="\(.*\)"$/=\1/;
	# Concatenate string values
	s/" *"//g;
	# Wrap non-numeral values with quotes
	s/=\(.*\?[^0-9].*\)$/=\"\1\"/;
	# Change '1' and empty values to "y" (not perfect, but
	# supports conditional compilation in the makefiles
	s/=$/=y/;
	s/=1$/=y/;
	# print the line
	p
}
