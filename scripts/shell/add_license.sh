#!/bin/sh

UPDATE_OROCOS_LICENSE=yes
REPLACE_GPL=no
FOR_REAL=yes

toplevel=../..
search_string=" *  You should have received a copy of the GNU"
gpl_signature="GNU General"
orocos_license_signature="Components for robotics"

for file in `find $toplevel -name *.h`   \
            `find $toplevel -name *.cpp` \
            `find $toplevel -name *.c`   \
            `find $toplevel -name *.hpp` \
            `find $toplevel -name *.dox`
do

    if `grep -q "$search_string" $file`; then

        # Has a license 
        if [ "$UPDATE_OROCOS_LICENSE" == "yes" ] || [ "$REPLACE_GPL" == "yes" ]; then

            license_end_line=`grep --line-number --max-count=1 "*/" $file | sed s/:.*//`
            total_num_lines=`wc --lines $file | sed s/\ .*//`

            if [ "$REPLACE_GPL" == "yes" ]; then
                if `head $file --lines=$license_end_line | grep -q "$gpl_signature"`; then
                    cat license.txt > temp
                    cat $file | tail --lines=`expr $total_num_lines - $license_end_line` >> temp
                    num_licenses=`grep -c "$search_string" temp`
                    # echo " -- num_licenses: $num_licenses"
                    if [ $num_licenses -gt 1 ]; then
                        echo "Oops: $file ended up with multiple licenses..."
                    #exit 1
                    fi

                    echo "Replacing GPL license in $file"
                    if [ $FOR_REAL == "yes" ]; then
                        cat temp > $file
                    fi
                fi
            fi
            
            if [ "$UPDATE_OROCOS_LICENSE" == "yes" ]; then
                if `head $file --lines=$license_end_line | grep -q "$orocos_license_signature"`; then
                    cat license.txt > temp
                    cat $file | tail --lines=`expr $total_num_lines - $license_end_line` >> temp
                    num_licenses=`grep -c "$search_string" temp`
                    # echo " -- num_licenses: $num_licenses"
                    if [ $num_licenses -gt 1 ]; then
                        echo "Oops: $file ended up with multiple licenses..."
                    #exit 1
                    fi

                    echo "Updating OROCOS license in $file"
                    if [ $FOR_REAL == "yes" ]; then
                        cat temp > $file
                    fi
                fi
            fi
        fi

    else
        # No license found -- add one.
         echo "Adding license to $file"
         cat license.txt > temp
         cat $file >> temp
         if [ $FOR_REAL == "yes" ]; then
             cat temp > $file
         fi
    fi

done