#!/bin/bash

for name in $(ls *.res | cut -f1 -d_ | uniq)
do
	count=$(ls $name*.res | wc -l)
	if [ "$count" -eq 1 ]
	then
		max=$(wc -l $name*.res | sort | tail -n1 | sed -r 's/^([^.]+).*$/\1/; s/^[^0-9]*([0-9]+).*$/\1/')
	else
		max=$(wc -l $name*.res | head -n -1 | sort | tail -n1 | sed -r 's/^([^.]+).*$/\1/; s/^[^0-9]*([0-9]+).*$/\1/')
	fi
	for file in $name\_b\_asp.res $name\_c\_asp.res $name\_m\_asp.res $name\_p\_asp.res $name\_b\_asp-teg.res $name\_c\_asp-teg.res $name\_m\_asp-teg.res $name\_p\_asp-teg.res $name\_b\_sat.res $name\_c\_sat.res $name\_m\_sat.res $name\_p\_sat.res $name\_cbs.res
	do
		if [ -f "$file" ]
		then
			size=$(($max - $(wc -l $file | sed -r 's/^([^.]+).*$/\1/; s/^[^0-9]*([0-9]+).*$/\1/')))
		else
			size=$max
		fi
		#echo $file needs $size lines

		if echo "$file" | grep asp
		then
			for i in $(seq $size)
			do
				printf '\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\n' >> $file
			done
		fi

		if echo "$file" | grep sat
		then
			for i in $(seq $size)
			do
				printf '\t\t\t\t\t\t\t\n' >> $file
			done
		fi

		if echo "$file" | grep cbs
		then
			for i in $(seq $size)
			do
				printf '\t\t\t\n' >> $file
			done
		fi
	done

	paste $name\_b\_asp.res $name\_c\_asp.res $name\_m\_asp.res $name\_p\_asp.res $name\_b\_asp-teg.res $name\_c\_asp-teg.res $name\_m\_asp-teg.res $name\_p\_asp-teg.res $name\_b\_sat.res $name\_c\_sat.res $name\_m\_sat.res $name\_p\_sat.res $name\_cbs.res > $name.tmp
done

cat *.tmp > 0results.txt

rm -f *.tmp
