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
	for file in $name\_b\_asp-teg\_single.res $name\_c\_asp-teg\_single.res $name\_m\_asp-teg\_single.res $name\_p\_asp-teg\_single.res $name\_c\_asp-teg\_all.res $name\_m\_asp-teg\_all.res $name\_p\_asp-teg\_all.res $name\_c\_asp-teg\_diverse.res $name\_m\_asp-teg\_diverse.res $name\_p\_asp-teg\_diverse.res $name\_c\_asp-teg\_random.res $name\_m\_asp-teg\_random.res $name\_p\_asp-teg\_random.res
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
	done

	paste $name\_b\_asp-teg\_single.res $name\_c\_asp-teg\_single.res $name\_m\_asp-teg\_single.res $name\_p\_asp-teg\_single.res $name\_c\_asp-teg\_all.res $name\_m\_asp-teg\_all.res $name\_p\_asp-teg\_all.res $name\_c\_asp-teg\_diverse.res $name\_m\_asp-teg\_diverse.res $name\_p\_asp-teg\_diverse.res $name\_c\_asp-teg\_random.res $name\_m\_asp-teg\_random.res $name\_p\_asp-teg\_random.res > $name.tmp
done

cat *.tmp > 0results.txt

rm -f *.tmp
