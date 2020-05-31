#!/bin/bash
#!/bin/bash
today=`date '+%Y%m%d-%HH%MM%S'`;
logdir=`pwd`

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
	-i|--iterations)
	ITERATIONS="$2"
	shift # past argument
	shift # past value
	;;
	-s|--size)
	SIZE="$2"
	shift # past argument
	shift # past value
	;;
	-d|--directory)
	TESTDIR="$2"
	shift # past argument
	shift # past value
	;;
	*)    # unknown option
	POSITIONAL+=("$1") # save it in an array for later
	shift # past argument
	;;
esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

if test -z "$TESTDIR"
then
	TESTDIR="."
fi
if test -z "$ITERATIONS"
then
	ITERATIONS="100"
fi
if test -z "$SIZE"
then
	SIZE="100"
fi


cd $TESTDIR

logfile="$logdir/log-$today.txt"

git log -n 1 --pretty=format:'%h %d %s%n%n' > $logfile

echo "_________________________________________________________________________________________________" >> $logfile
echo "| Test File Name  |         Sub Test          | Iter | SIZE |  FPS   |Average | Median | Sigma  |" >> $logfile
echo "|-----------------|---------------------------|------|------|--------|--------|--------|--------|" >> $logfile
for file in test_*
do
	./"$file" $ITERATIONS $SIZE >> $logfile
done
