#!/usr/bin/env bash

function build() {
  CFLAGS="$1" pebble build
  pebble wipe
}

function screenshot() {
  pebble install --emulator "$1"
  sleep 2

  if [[ "$3" == "true" ]]; then
    pebble emu-set-timeline-quick-view --emulator "$1" on
  fi

  pebble screenshot --emulator "$1" ./img/screenshots/"$2".png
  if [[ "$3" == "true" ]]; then
    pebble emu-set-timeline-quick-view --emulator "$1" off
  fi
  pebble kill
}

build "-DHOUR=22 -DMINUTE=6 -DSECOND=30 -DDAY=6 -DDATE=6"
screenshot diorite diorite1
screenshot basalt basalt1
screenshot chalk chalk1

build "-DHOUR=22 -DMINUTE=6 -DSECOND=30 -DDAY=6 -DDATE=6 -DDIGITAL=true"
screenshot diorite diorite2
screenshot basalt basalt2
screenshot chalk chalk2

build "-DHOUR=22 -DMINUTE=6 -DSECOND=30 -DDAY=6 -DDATE=6 -DFONT=2 -DBGC1=GColorRed -DHHC=GColorOrange -DMHC=GColorOrange"
screenshot diorite diorite3
screenshot basalt basalt3
screenshot chalk chalk3

build "-DHOUR=22 -DMINUTE=6 -DSECOND=30 -DDAY=6 -DDATE=6 -DFONT=3 -DBGC1=GColorBlack -DBGC2=GColorBlack -DTC1=GColorWhite -DTC2=GColorWhite -DHHC=GColorBlack -DMHC=GColorBlack -DHHBC=GColorWhite -DMHBC=GColorWhite -DSHC=GColorWhite -DBWBGC1=GColorBlack -DBWTC1=GColorWhite"
screenshot diorite diorite4
screenshot basalt basalt4
screenshot chalk chalk4
