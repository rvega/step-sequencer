////////////////////////////////////////////////////////////////////////////////
// 
//  Sequencer
//  Copyright (C) 2015 Rafael Vega <rvega@elsoftwarehamuerto.org>
//  Copyright (C) 2015 Miguel Vargas <miguelito.vargasf@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify it 
//  under the terms of the GNU General Public License as published by the Free 
//  Software Foundation, either version 3 of the License, or (at your option) 
//  any later version.
//  
//  This program is distributed in the hope that it will be useful, but WITHOUT 
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
//  more details.  
//  
//  You should have received a copy of the GNU General Public License along 
//  with this program.  If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////

#include "m_pd.h"
#include "s_stuff.h" // more pd stuff 

/* #include <string.h> */
/* #include <stdlib.h> */
/* #include <stdio.h> */

////////////////////////////////////////////////////////////////////////////////
// Data
//

typedef struct sequencer {
   t_object x_obj;
   t_outlet *outlet0;
   t_outlet *outlet1;

   t_clock *clock;
   t_float tempo;
   int current_beat;

   int current_instrument;
} t_sequencer;

// A pointer to the class object.
t_class *sequencer_class;


////////////////////////////////////////////////////////////////////////////////
// Received PD Messages
// 

// Received "switch-instrument" message.
static void sequencer_switch_instrument(t_sequencer* x) {
   // Turn off previous instrument
   t_atom list[2];
   SETFLOAT(list, 200 + x->current_instrument);
   SETFLOAT(list+1, 0);
   outlet_list(x->outlet1, gensym("list"), 2, list);

   // Turn on current instrument
   x->current_instrument ++;
   x->current_instrument %= 4;

   SETFLOAT(list, 200+x->current_instrument);
   SETFLOAT(list+1, 1);
   outlet_list(x->outlet1, gensym("list"), 2, list);
}

// Received "tempo" message, with parameter
static void sequencer_tempo(t_sequencer* x, t_float f) {
   x->tempo = f;
   clock_delay(x->clock, x->tempo);
}

// Received "button" message, with parameter
static void sequencer_button(t_sequencer* x, t_float f) {
   outlet_float(x->outlet1, f);
}

///////////////////////////////////////////////////////////////////////////////
// Time
//
static void clock_tick(t_sequencer *x){
   clock_delay(x->clock, x->tempo);

   // Turn off current led
   t_atom list[2];
   SETFLOAT(list, x->current_beat + 100);
   SETFLOAT(list+1, 0);
   outlet_list(x->outlet1, gensym("list"), 2, list);

   x->current_beat++;
   x->current_beat %= 16;

   // Turn on current (next) led
   SETFLOAT(list, x->current_beat + 100);
   SETFLOAT(list+1, 1);
   outlet_list(x->outlet1, gensym("list"), 2, list);
}

///////////////////////////////////////////////////////////////////////////////
// Constructor, destructor
//

static void sequencer_loadbang(t_sequencer *x){
   if (!sys_noloadbang){
      // Turn on first instrument
      t_atom list[2];
      SETFLOAT(list, 200+x->current_instrument);
      SETFLOAT(list+1, 1);
      outlet_list(x->outlet1, gensym("list"), 2, list);
   }
   (void)x;
}

static void *sequencer_new(void) {
   t_sequencer *x = (t_sequencer *)pd_new(sequencer_class);

   x->outlet0 = outlet_new(&x->x_obj, &s_anything);
   x->outlet1= outlet_new(&x->x_obj, &s_anything);
   
   x->clock = clock_new(x, (t_method)clock_tick);
   x->tempo = 1000;
   x->current_beat = 0;
   clock_delay(x->clock, x->tempo);

   x->current_instrument = 0;

   return (void *)x;
}

static void sequencer_free(t_sequencer *x) { 
   clock_free(x->clock);
}

///////////////////////////////////////////////////////////////////////////////
// Class definition
// 


void sequencer_setup(void) {
   sequencer_class = class_new(gensym("sequencer"), (t_newmethod)sequencer_new, (t_method)sequencer_free, sizeof(t_sequencer), CLASS_DEFAULT, (t_atomtype)0);
   class_addmethod(sequencer_class, (t_method)sequencer_switch_instrument, gensym("switch-instrument"), 0);
   class_addmethod(sequencer_class, (t_method)sequencer_tempo, gensym("tempo"), A_DEFFLOAT, 0);
   class_addmethod(sequencer_class, (t_method)sequencer_button, gensym("button"), A_DEFFLOAT, 0);
   class_addmethod(sequencer_class, (t_method)sequencer_loadbang, gensym("loadbang"), 0);
}
