#!/bin/bash

for i in `seq 10 -1 1`; do
	echo -n "$i($$).."
	sleep 1
done

echo "Blastoff $$ Complete"

