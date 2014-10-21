ifeq ($(VERBOSE),on)
  echo=:
else
  echo=echo
  .SILENT:
endif


ifeq ($(COLOR),on)
cleaning=$(echo)	"  [01;34mcleaning[00m"
assembling=$(echo)	"  [01;32massembling[00m"
compiling=$(echo)	"  [01;32mcompiling[00m"
generating=$(echo)	"  [01;32mgenerating[00m"
linking=$(echo)		"  [01;32mlinking[00m"
else
cleaning=$(echo)	"  cleaning"
assembling=$(echo)	"  assembling"
compiling=$(echo)	"  compiling"
generating=$(echo)	"  generating"
linking=$(echo)		"  linking"
endif
