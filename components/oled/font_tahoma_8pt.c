/*
 *font_tahoma_8pt.c
 *
 *  Created on: Jan 3, 2015
 *      Author: Baoshi
 */
//#include "esp_common.h"
#include "fonts.h"

/*
**  Font data for Tahoma 8pt
*/

/* Character bitmaps for Tahoma 8pt */
const uint8_t tahoma_8pt_bitmaps[] =
{
    /* @0 ' ' (1 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //

    /* @11 '!' (1 pixels wide) */
    0x00, //
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x00, //
    0x80, // #
    0x00, //
    0x00, //

    /* @22 '"' (3 pixels wide) */
    0xA0, // # #
    0xA0, // # #
    0xA0, // # #
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //

    /* @33 '#' (7 pixels wide) */
    0x00, //
    0x14, //    # #
    0x14, //    # #
    0x7E, //  ######
    0x28, //   # #
    0x28, //   # #
    0xFC, // ######
    0x50, //  # #
    0x50, //  # #
    0x00, //
    0x00, //

    /* @44 '$' (5 pixels wide) */
    0x20, //   #
    0x20, //   #
    0x78, //  ####
    0xA0, // # #
    0xA0, // # #
    0x70, //  ###
    0x28, //   # #
    0x28, //   # #
    0xF0, // ####
    0x20, //   #
    0x20, //   #

    /* @55 '%' (10 pixels wide) */
    0x00, 0x00, //
    0x62, 0x00, //  ##   #
    0x92, 0x00, // #  #  #
    0x94, 0x00, // #  # #
    0x64, 0x00, //  ##  #
    0x09, 0x80, //     #  ##
    0x0A, 0x40, //     # #  #
    0x12, 0x40, //    #  #  #
    0x11, 0x80, //    #   ##
    0x00, 0x00, //
    0x00, 0x00, //

    /* @77 '&' (7 pixels wide) */
    0x00, //
    0x60, //  ##
    0x90, // #  #
    0x90, // #  #
    0x64, //  ##  #
    0x94, // #  # #
    0x88, // #   #
    0x8C, // #   ##
    0x72, //  ###  #
    0x00, //
    0x00, //

    /* @88 ''' (1 pixels wide) */
    0x80, // #
    0x80, // #
    0x80, // #
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //

    /* @99 '(' (3 pixels wide) */
    0x20, //   #
    0x40, //  #
    0x40, //  #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x40, //  #
    0x40, //  #
    0x20, //   #

    /* @110 ')' (3 pixels wide) */
    0x80, // #
    0x40, //  #
    0x40, //  #
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0x40, //  #
    0x40, //  #
    0x80, // #

    /* @121 '*' (5 pixels wide) */
    0x20, //   #
    0xA8, // # # #
    0x70, //  ###
    0xA8, // # # #
    0x20, //   #
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //

    /* @132 '+' (7 pixels wide) */
    0x00, //
    0x00, //
    0x10, //    #
    0x10, //    #
    0x10, //    #
    0xFE, // #######
    0x10, //    #
    0x10, //    #
    0x10, //    #
    0x00, //
    0x00, //

    /* @143 ',' (2 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x80, // #

    /* @154 '-' (3 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0xE0, // ###
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //

    /* @165 '.' (1 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x80, // #
    0x80, // #
    0x00, //
    0x00, //

    /* @176 '/' (3 pixels wide) */
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x80, // #
    0x80, // #
    0x80, // #

    /* @187 '0' (5 pixels wide) */
    0x00, //
    0x70, //  ###
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x70, //  ###
    0x00, //
    0x00, //

    /* @198 '1' (3 pixels wide) */
    0x00, //
    0x40, //  #
    0xC0, // ##
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0xE0, // ###
    0x00, //
    0x00, //

    /* @209 '2' (5 pixels wide) */
    0x00, //
    0x70, //  ###
    0x88, // #   #
    0x08, //     #
    0x10, //    #
    0x20, //   #
    0x40, //  #
    0x80, // #
    0xF8, // #####
    0x00, //
    0x00, //

    /* @220 '3' (5 pixels wide) */
    0x00, //
    0x70, //  ###
    0x88, // #   #
    0x08, //     #
    0x30, //   ##
    0x08, //     #
    0x08, //     #
    0x88, // #   #
    0x70, //  ###
    0x00, //
    0x00, //

    /* @231 '4' (5 pixels wide) */
    0x00, //
    0x10, //    #
    0x30, //   ##
    0x50, //  # #
    0x90, // #  #
    0xF8, // #####
    0x10, //    #
    0x10, //    #
    0x10, //    #
    0x00, //
    0x00, //

    /* @242 '5' (5 pixels wide) */
    0x00, //
    0xF8, // #####
    0x80, // #
    0x80, // #
    0xF0, // ####
    0x08, //     #
    0x08, //     #
    0x88, // #   #
    0x70, //  ###
    0x00, //
    0x00, //

    /* @253 '6' (5 pixels wide) */
    0x00, //
    0x30, //   ##
    0x40, //  #
    0x80, // #
    0xF0, // ####
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x70, //  ###
    0x00, //
    0x00, //

    /* @264 '7' (5 pixels wide) */
    0x00, //
    0xF8, // #####
    0x08, //     #
    0x10, //    #
    0x10, //    #
    0x20, //   #
    0x20, //   #
    0x40, //  #
    0x40, //  #
    0x00, //
    0x00, //

    /* @275 '8' (5 pixels wide) */
    0x00, //
    0x70, //  ###
    0x88, // #   #
    0x88, // #   #
    0x70, //  ###
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x70, //  ###
    0x00, //
    0x00, //

    /* @286 '9' (5 pixels wide) */
    0x00, //
    0x70, //  ###
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x78, //  ####
    0x08, //     #
    0x10, //    #
    0x60, //  ##
    0x00, //
    0x00, //

    /* @297 ':' (1 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x80, // #
    0x80, // #
    0x00, //
    0x00, //
    0x80, // #
    0x80, // #
    0x00, //
    0x00, //

    /* @308 ';' (2 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x40, //  #
    0x40, //  #
    0x00, //
    0x00, //
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x80, // #

    /* @319 '<' (6 pixels wide) */
    0x00, //
    0x00, //
    0x04, //      #
    0x18, //    ##
    0x60, //  ##
    0x80, // #
    0x60, //  ##
    0x18, //    ##
    0x04, //      #
    0x00, //
    0x00, //

    /* @330 '=' (7 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0xFE, // #######
    0x00, //
    0xFE, // #######
    0x00, //
    0x00, //
    0x00, //
    0x00, //

    /* @341 '>' (6 pixels wide) */
    0x00, //
    0x00, //
    0x80, // #
    0x60, //  ##
    0x18, //    ##
    0x04, //      #
    0x18, //    ##
    0x60, //  ##
    0x80, // #
    0x00, //
    0x00, //

    /* @352 '?' (4 pixels wide) */
    0x00, //
    0xE0, // ###
    0x10, //    #
    0x10, //    #
    0x20, //   #
    0x40, //  #
    0x40, //  #
    0x00, //
    0x40, //  #
    0x00, //
    0x00, //

    /* @363 '@' (9 pixels wide) */
    0x00, 0x00, //
    0x3E, 0x00, //   #####
    0x41, 0x00, //  #     #
    0x9C, 0x80, // #  ###  #
    0xA4, 0x80, // # #  #  #
    0xA4, 0x80, // # #  #  #
    0xA4, 0x80, // # #  #  #
    0x9F, 0x00, // #  #####
    0x40, 0x00, //  #
    0x3C, 0x00, //   ####
    0x00, 0x00, //

    /* @385 'A' (6 pixels wide) */
    0x00, //
    0x30, //   ##
    0x30, //   ##
    0x48, //  #  #
    0x48, //  #  #
    0x48, //  #  #
    0xFC, // ######
    0x84, // #    #
    0x84, // #    #
    0x00, //
    0x00, //

    /* @396 'B' (5 pixels wide) */
    0x00, //
    0xF0, // ####
    0x88, // #   #
    0x88, // #   #
    0xF0, // ####
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0xF0, // ####
    0x00, //
    0x00, //

    /* @407 'C' (6 pixels wide) */
    0x00, //
    0x3C, //   ####
    0x40, //  #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x40, //  #
    0x3C, //   ####
    0x00, //
    0x00, //

    /* @418 'D' (6 pixels wide) */
    0x00, //
    0xF0, // ####
    0x88, // #   #
    0x84, // #    #
    0x84, // #    #
    0x84, // #    #
    0x84, // #    #
    0x88, // #   #
    0xF0, // ####
    0x00, //
    0x00, //

    /* @429 'E' (5 pixels wide) */
    0x00, //
    0xF8, // #####
    0x80, // #
    0x80, // #
    0xF0, // ####
    0x80, // #
    0x80, // #
    0x80, // #
    0xF8, // #####
    0x00, //
    0x00, //

    /* @440 'F' (5 pixels wide) */
    0x00, //
    0xF8, // #####
    0x80, // #
    0x80, // #
    0xF8, // #####
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x00, //
    0x00, //

    /* @451 'G' (6 pixels wide) */
    0x00, //
    0x3C, //   ####
    0x40, //  #
    0x80, // #
    0x80, // #
    0x9C, // #  ###
    0x84, // #    #
    0x44, //  #   #
    0x3C, //   ####
    0x00, //
    0x00, //

    /* @462 'H' (6 pixels wide) */
    0x00, //
    0x84, // #    #
    0x84, // #    #
    0x84, // #    #
    0xFC, // ######
    0x84, // #    #
    0x84, // #    #
    0x84, // #    #
    0x84, // #    #
    0x00, //
    0x00, //

    /* @473 'I' (3 pixels wide) */
    0x00, //
    0xE0, // ###
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0xE0, // ###
    0x00, //
    0x00, //

    /* @484 'J' (4 pixels wide) */
    0x00, //
    0x70, //  ###
    0x10, //    #
    0x10, //    #
    0x10, //    #
    0x10, //    #
    0x10, //    #
    0x10, //    #
    0xE0, // ###
    0x00, //
    0x00, //

    /* @495 'K' (5 pixels wide) */
    0x00, //
    0x88, // #   #
    0x90, // #  #
    0xA0, // # #
    0xC0, // ##
    0xC0, // ##
    0xA0, // # #
    0x90, // #  #
    0x88, // #   #
    0x00, //
    0x00, //

    /* @506 'L' (4 pixels wide) */
    0x00, //
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0xF0, // ####
    0x00, //
    0x00, //

    /* @517 'M' (7 pixels wide) */
    0x00, //
    0xC6, // ##   ##
    0xC6, // ##   ##
    0xAA, // # # # #
    0xAA, // # # # #
    0x92, // #  #  #
    0x92, // #  #  #
    0x82, // #     #
    0x82, // #     #
    0x00, //
    0x00, //

    /* @528 'N' (6 pixels wide) */
    0x00, //
    0xC4, // ##   #
    0xC4, // ##   #
    0xA4, // # #  #
    0xA4, // # #  #
    0x94, // #  # #
    0x94, // #  # #
    0x8C, // #   ##
    0x8C, // #   ##
    0x00, //
    0x00, //

    /* @539 'O' (7 pixels wide) */
    0x00, //
    0x38, //   ###
    0x44, //  #   #
    0x82, // #     #
    0x82, // #     #
    0x82, // #     #
    0x82, // #     #
    0x44, //  #   #
    0x38, //   ###
    0x00, //
    0x00, //

    /* @550 'P' (5 pixels wide) */
    0x00, //
    0xF0, // ####
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0xF0, // ####
    0x80, // #
    0x80, // #
    0x80, // #
    0x00, //
    0x00, //

    /* @561 'Q' (7 pixels wide) */
    0x00, //
    0x38, //   ###
    0x44, //  #   #
    0x82, // #     #
    0x82, // #     #
    0x82, // #     #
    0x82, // #     #
    0x44, //  #   #
    0x38, //   ###
    0x08, //     #
    0x06, //      ##

    /* @572 'R' (6 pixels wide) */
    0x00, //
    0xF0, // ####
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0xF0, // ####
    0x90, // #  #
    0x88, // #   #
    0x84, // #    #
    0x00, //
    0x00, //

    /* @583 'S' (5 pixels wide) */
    0x00, //
    0x78, //  ####
    0x80, // #
    0x80, // #
    0x70, //  ###
    0x08, //     #
    0x08, //     #
    0x08, //     #
    0xF0, // ####
    0x00, //
    0x00, //

    /* @594 'T' (5 pixels wide) */
    0x00, //
    0xF8, // #####
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0x00, //
    0x00, //

    /* @605 'U' (6 pixels wide) */
    0x00, //
    0x84, // #    #
    0x84, // #    #
    0x84, // #    #
    0x84, // #    #
    0x84, // #    #
    0x84, // #    #
    0x84, // #    #
    0x78, //  ####
    0x00, //
    0x00, //

    /* @616 'V' (5 pixels wide) */
    0x00, //
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x50, //  # #
    0x50, //  # #
    0x50, //  # #
    0x20, //   #
    0x20, //   #
    0x00, //
    0x00, //

    /* @627 'W' (9 pixels wide) */
    0x00, 0x00, //
    0x88, 0x80, // #   #   #
    0x88, 0x80, // #   #   #
    0x88, 0x80, // #   #   #
    0x55, 0x00, //  # # # #
    0x55, 0x00, //  # # # #
    0x55, 0x00, //  # # # #
    0x22, 0x00, //   #   #
    0x22, 0x00, //   #   #
    0x00, 0x00, //
    0x00, 0x00, //

    /* @649 'X' (5 pixels wide) */
    0x00, //
    0x88, // #   #
    0x88, // #   #
    0x50, //  # #
    0x20, //   #
    0x20, //   #
    0x50, //  # #
    0x88, // #   #
    0x88, // #   #
    0x00, //
    0x00, //

    /* @660 'Y' (5 pixels wide) */
    0x00, //
    0x88, // #   #
    0x88, // #   #
    0x50, //  # #
    0x50, //  # #
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0x00, //
    0x00, //

    /* @671 'Z' (5 pixels wide) */
    0x00, //
    0xF8, // #####
    0x08, //     #
    0x10, //    #
    0x20, //   #
    0x20, //   #
    0x40, //  #
    0x80, // #
    0xF8, // #####
    0x00, //
    0x00, //

    /* @682 '[' (3 pixels wide) */
    0xE0, // ###
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0xE0, // ###

    /* @693 '\' (3 pixels wide) */
    0x80, // #
    0x80, // #
    0x80, // #
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x20, //   #
    0x20, //   #
    0x20, //   #

    /* @704 ']' (3 pixels wide) */
    0xE0, // ###
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0xE0, // ###

    /* @715 '^' (7 pixels wide) */
    0x00, //
    0x10, //    #
    0x28, //   # #
    0x44, //  #   #
    0x82, // #     #
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //

    /* @726 '_' (6 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0xFC, // ######

    /* @737 '`' (2 pixels wide) */
    0x80, // #
    0x40, //  #
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x00, //

    /* @748 'a' (5 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x70, //  ###
    0x08, //     #
    0x78, //  ####
    0x88, // #   #
    0x88, // #   #
    0x78, //  ####
    0x00, //
    0x00, //

    /* @759 'b' (5 pixels wide) */
    0x80, // #
    0x80, // #
    0x80, // #
    0xF0, // ####
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0xF0, // ####
    0x00, //
    0x00, //

    /* @770 'c' (4 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x70, //  ###
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x70, //  ###
    0x00, //
    0x00, //

    /* @781 'd' (5 pixels wide) */
    0x08, //     #
    0x08, //     #
    0x08, //     #
    0x78, //  ####
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x78, //  ####
    0x00, //
    0x00, //

    /* @792 'e' (5 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x70, //  ###
    0x88, // #   #
    0xF8, // #####
    0x80, // #
    0x88, // #   #
    0x70, //  ###
    0x00, //
    0x00, //

    /* @803 'f' (3 pixels wide) */
    0x60, //  ##
    0x80, // #
    0x80, // #
    0xE0, // ###
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x00, //
    0x00, //

    /* @814 'g' (5 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x78, //  ####
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x78, //  ####
    0x08, //     #
    0x70, //  ###

    /* @825 'h' (5 pixels wide) */
    0x80, // #
    0x80, // #
    0x80, // #
    0xF0, // ####
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x00, //
    0x00, //

    /* @836 'i' (1 pixels wide) */
    0x00, //
    0x80, // #
    0x00, //
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x00, //
    0x00, //

    /* @847 'j' (2 pixels wide) */
    0x00, //
    0x40, //  #
    0x00, //
    0xC0, // ##
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x80, // #

    /* @858 'k' (5 pixels wide) */
    0x80, // #
    0x80, // #
    0x80, // #
    0x90, // #  #
    0xA0, // # #
    0xC0, // ##
    0xA0, // # #
    0x90, // #  #
    0x88, // #   #
    0x00, //
    0x00, //

    /* @869 'l' (1 pixels wide) */
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x00, //
    0x00, //

    /* @880 'm' (7 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0xEC, // ### ##
    0x92, // #  #  #
    0x92, // #  #  #
    0x92, // #  #  #
    0x92, // #  #  #
    0x92, // #  #  #
    0x00, //
    0x00, //

    /* @891 'n' (5 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0xF0, // ####
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x00, //
    0x00, //

    /* @902 'o' (5 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x70, //  ###
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x70, //  ###
    0x00, //
    0x00, //

    /* @913 'p' (5 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0xF0, // ####
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0xF0, // ####
    0x80, // #
    0x80, // #

    /* @924 'q' (5 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x78, //  ####
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x78, //  ####
    0x08, //     #
    0x08, //     #

    /* @935 'r' (3 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0xA0, // # #
    0xC0, // ##
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x00, //
    0x00, //

    /* @946 's' (4 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x70, //  ###
    0x80, // #
    0xC0, // ##
    0x30, //   ##
    0x10, //    #
    0xE0, // ###
    0x00, //
    0x00, //

    /* @957 't' (3 pixels wide) */
    0x00, //
    0x80, // #
    0x80, // #
    0xE0, // ###
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x60, //  ##
    0x00, //
    0x00, //

    /* @968 'u' (5 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x88, // #   #
    0x78, //  ####
    0x00, //
    0x00, //

    /* @979 'v' (5 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x88, // #   #
    0x88, // #   #
    0x50, //  # #
    0x50, //  # #
    0x20, //   #
    0x20, //   #
    0x00, //
    0x00, //

    /* @990 'w' (7 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x92, // #  #  #
    0x92, // #  #  #
    0xAA, // # # # #
    0xAA, // # # # #
    0x44, //  #   #
    0x44, //  #   #
    0x00, //
    0x00, //

    /* @1001 'x' (5 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x88, // #   #
    0x50, //  # #
    0x20, //   #
    0x20, //   #
    0x50, //  # #
    0x88, // #   #
    0x00, //
    0x00, //

    /* @1012 'y' (5 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x88, // #   #
    0x88, // #   #
    0x50, //  # #
    0x50, //  # #
    0x20, //   #
    0x20, //   #
    0x40, //  #
    0x40, //  #

    /* @1023 'z' (4 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0xF0, // ####
    0x10, //    #
    0x20, //   #
    0x40, //  #
    0x80, // #
    0xF0, // ####
    0x00, //
    0x00, //

    /* @1034 '{' (4 pixels wide) */
    0x10, //    #
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0xC0, // ##
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0x20, //   #
    0x10, //    #

    /* @1045 '|' (1 pixels wide) */
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #
    0x80, // #

    /* @1056 '}' (4 pixels wide) */
    0x80, // #
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x30, //   ##
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x40, //  #
    0x80, // #

    /* @1067 '~' (7 pixels wide) */
    0x00, //
    0x00, //
    0x00, //
    0x00, //
    0x62, //  ##   #
    0x92, // #  #  #
    0x8C, // #   ##
    0x00, //
    0x00, //
    0x00, //
    0x00, //
};

/* Character descriptors for Tahoma 8pt */
/* { [Char width in bits], [Offset into tahoma_8ptCharBitmaps in bytes] } */
const font_char_desc_t tahoma_descriptors[] =
{
    {1, 0},         /*   */
    {1, 11},        /* ! */
    {3, 22},        /* " */
    {7, 33},        /* # */
    {5, 44},        /* $ */
    {10, 55},       /* % */
    {7, 77},        /* & */
    {1, 88},        /* ' */
    {3, 99},        /* ( */
    {3, 110},       /* ) */
    {5, 121},       /* * */
    {7, 132},       /* + */
    {2, 143},       /* , */
    {3, 154},       /* - */
    {1, 165},       /* . */
    {3, 176},       /* / */
    {5, 187},       /* 0 */
    {3, 198},       /* 1 */
    {5, 209},       /* 2 */
    {5, 220},       /* 3 */
    {5, 231},       /* 4 */
    {5, 242},       /* 5 */
    {5, 253},       /* 6 */
    {5, 264},       /* 7 */
    {5, 275},       /* 8 */
    {5, 286},       /* 9 */
    {1, 297},       /* : */
    {2, 308},       /* ; */
    {6, 319},       /* < */
    {7, 330},       /* = */
    {6, 341},       /* > */
    {4, 352},       /* ? */
    {9, 363},       /* @ */
    {6, 385},       /* A */
    {5, 396},       /* B */
    {6, 407},       /* C */
    {6, 418},       /* D */
    {5, 429},       /* E */
    {5, 440},       /* F */
    {6, 451},       /* G */
    {6, 462},       /* H */
    {3, 473},       /* I */
    {4, 484},       /* J */
    {5, 495},       /* K */
    {4, 506},       /* L */
    {7, 517},       /* M */
    {6, 528},       /* N */
    {7, 539},       /* O */
    {5, 550},       /* P */
    {7, 561},       /* Q */
    {6, 572},       /* R */
    {5, 583},       /* S */
    {5, 594},       /* T */
    {6, 605},       /* U */
    {5, 616},       /* V */
    {9, 627},       /* W */
    {5, 649},       /* X */
    {5, 660},       /* Y */
    {5, 671},       /* Z */
    {3, 682},       /* [ */
    {3, 693},       /* \ */
    {3, 704},       /* ] */
    {7, 715},       /* ^ */
    {6, 726},       /* _ */
    {2, 737},       /* ` */
    {5, 748},       /* a */
    {5, 759},       /* b */
    {4, 770},       /* c */
    {5, 781},       /* d */
    {5, 792},       /* e */
    {3, 803},       /* f */
    {5, 814},       /* g */
    {5, 825},       /* h */
    {1, 836},       /* i */
    {2, 847},       /* j */
    {5, 858},       /* k */
    {1, 869},       /* l */
    {7, 880},       /* m */
    {5, 891},       /* n */
    {5, 902},       /* o */
    {5, 913},       /* p */
    {5, 924},       /* q */
    {3, 935},       /* r */
    {4, 946},       /* s */
    {3, 957},       /* t */
    {5, 968},       /* u */
    {5, 979},       /* v */
    {7, 990},       /* w */
    {5, 1001},      /* x */
    {5, 1012},      /* y */
    {4, 1023},      /* z */
    {4, 1034},      /* { */
    {1, 1045},      /* | */
    {4, 1056},      /* } */
    {7, 1067},      /* ~ */
};

/* Font information for Tahoma 8pt */
const font_info_t tahoma_8pt_font_info =
{
    11, /*  Character height */
    1,  /*  C */
    ' ', /*  Start character */
    '~', /*  End character */
    tahoma_descriptors, /*  Character descriptor array */
    tahoma_8pt_bitmaps, /*  Character bitmap array */
};