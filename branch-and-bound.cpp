//
//  branch-and-bound.cpp
//  SofaBounds Version 1.0
//
//  This source file is part of the SofaBounds software package
//  SofaBounds is a program that proves upper bounds in the moving sofa problem,
//  as described in the paper "Improved upper bounds in the moving sofa problem",
//  by Yoav Kallus and Dan Romik
//
//  For more information go to the project web page: *** TODO: add URL ***
//
//  Copyright © 2017 Yoav Kallus and Dan Romik
//

#include <queue>
#include <vector>
#include <fstream>
#include "sofa-bounds.hpp"

int branch_and_bound(struct bb_thread_params *my_bb_thread_params) {
    unsigned int i,j;
    std::priority_queue<struct box, std::vector<struct box>, struct CompareBoxes> boxqueue;
    struct box currentbox;
    struct box newbox[2];
    ExactRational newbound[2];
    ExactRational a,b,c,d;
    ExactRational bestyet;
    long elapsed_ms = 0;

    // Try to load checkpoint if it exists
    bool loaded_checkpoint = false;
    if (!my_bb_thread_params->checkpoint_filename.empty()) {
        loaded_checkpoint = load_checkpoint(my_bb_thread_params->checkpoint_filename, my_bb_thread_params, boxqueue, elapsed_ms);
        if (loaded_checkpoint) {
            std::cout << "Loaded checkpoint from '" << my_bb_thread_params->checkpoint_filename << "'" << std::endl;
            std::cout << "Resuming from iteration " << my_bb_thread_params->iterations << std::endl;
            // Adjust start time to account for elapsed time
            my_bb_thread_params->t_start = std::chrono::high_resolution_clock::now() - std::chrono::milliseconds(elapsed_ms);
            my_bb_thread_params->checkpoint_iter_last = my_bb_thread_params->iterations;
            bestyet = my_bb_thread_params->lower_bound;
        }
    }

    if (!loaded_checkpoint) {
        //initialize lower bound
        bestyet = my_bb_thread_params->lower_bound;

        //initailize the queue
        newbox[0] = a_priori_bounds(*my_bb_thread_params,my_bb_thread_params->num_intermediate/2);
        newbox[0].upper_bound_on_max_in_box = area_of_union(newbox[0],*my_bb_thread_params);
        boxqueue.push(newbox[0]);
        
        //initialize interation counter and vector recording the lower bound witness
        my_bb_thread_params->iterations = 0UL;
        my_bb_thread_params->lower_bound_witness.resize(2*my_bb_thread_params->num_intermediate);
        my_bb_thread_params->checkpoint_iter_last = 0;
    }

    //main branch-and-bound loop
    while (!boxqueue.empty() && !my_bb_thread_params->stop_flag) {
	my_bb_thread_params->iterations++;

        //pop top box from the stack, and declare its upper bound the universal upper bound
        currentbox = boxqueue.top();
        boxqueue.pop();
	my_bb_thread_params->upper_bound = currentbox.upper_bound_on_max_in_box;

	//if requested, construct polygons corresponding to the current best lower bound and upper bound
	//and print their vertex coordinates to a file
	if (my_bb_thread_params->report_flag) {
	    //upper bound polygon
	    //first line is number of vertices x 2
	    polygon_of_union(currentbox,*my_bb_thread_params,&my_bb_thread_params->upper_bound_polygon);
	    std::ofstream fpoly(my_bb_thread_params->fpoly_name);
	    fpoly << my_bb_thread_params->upper_bound_polygon.size() << std::endl;
	    for (i=0; i<my_bb_thread_params->upper_bound_polygon.size();i++)
		//each subsequent line alternates between x and y coordinates of successive vertices
		fpoly << rat_str(my_bb_thread_params->upper_bound_polygon[i]) << std::endl;
	    //lower bound polygon
	    fpoly << my_bb_thread_params->lower_bound_polygon.size() << std::endl;
	    for (i=0; i<my_bb_thread_params->lower_bound_polygon.size();i++)
		fpoly << rat_str(my_bb_thread_params->lower_bound_polygon[i]) << std::endl;
	    fpoly.close();
	    my_bb_thread_params->report_flag = false;
	}

        //if local lower bound on max in current box is better than current global bound, update global bound
        a = lower_bound_on_max_in_box(currentbox,*my_bb_thread_params);
        if (a > bestyet) {
	    bestyet = a;
	    my_bb_thread_params->lower_bound = bestyet;
	    newbox[0] = currentbox;
	    //also record the witness variables and the associated polygon
	    for (i=0;i<2*my_bb_thread_params->num_intermediate;i++) {
		newbox[0].coord_bound_intervals[i] = midpoint_of_interval(currentbox.coord_bound_intervals[i]);
		my_bb_thread_params->lower_bound_witness[i] = newbox[0].coord_bound_intervals[i].left;
	    }
	    polygon_of_union(newbox[0],*my_bb_thread_params,&my_bb_thread_params->lower_bound_polygon);
	}

	if (my_bb_thread_params->reporteverysec_on) {
	    std::chrono::high_resolution_clock::time_point t_end = std::chrono::high_resolution_clock::now();
	    //long ms = std::chrono::duration_cast<std::chrono::milliseconds>(t_end-my_bb_thread_params->reporteverysec_last).count();
	    //if (ms >= 1000*my_bb_thread_params->reporteverysec_inc) {
	    if (t_end - my_bb_thread_params->reporteverysec_last >= std::chrono::seconds(my_bb_thread_params->reporteverysec_inc)) {
		std::cout << std::endl;
		short_inspect(my_bb_thread_params);
		std::cout << std::flush;
		//my_bb_thread_params->reporteverysec_last = t_end;
	        while (t_end - my_bb_thread_params->reporteverysec_last >= std::chrono::seconds(my_bb_thread_params->reporteverysec_inc)) my_bb_thread_params->reporteverysec_last += std::chrono::seconds(my_bb_thread_params->reporteverysec_inc);

	    }
	}
	if (my_bb_thread_params->reporteveryiter_on) {
	    if (my_bb_thread_params->iterations - my_bb_thread_params->reporteveryiter_last >= my_bb_thread_params->reporteveryiter_inc) {
		std::cout << std::endl;
		short_inspect(my_bb_thread_params);
		std::cout << std::flush;
		my_bb_thread_params->reporteveryiter_last = (my_bb_thread_params->iterations/my_bb_thread_params->reporteveryiter_inc)*my_bb_thread_params->reporteveryiter_inc;
	    }
	}
	if (my_bb_thread_params->reporteveryjump_on) {
	    if (my_bb_thread_params->reporteveryjump_last - CGAL::to_double(my_bb_thread_params->upper_bound) >= my_bb_thread_params->reporteveryjump_inc) {
		std::cout << std::endl;
		short_inspect(my_bb_thread_params);
		std::cout << std::flush;
		my_bb_thread_params->reporteveryjump_last = round(CGAL::to_double(my_bb_thread_params->upper_bound)/my_bb_thread_params->reporteveryjump_inc)*my_bb_thread_params->reporteveryjump_inc;
	    }
	}

	// Save checkpoint if enabled and enough iterations have passed
	if (my_bb_thread_params->checkpoint_on && 
	    my_bb_thread_params->iterations - my_bb_thread_params->checkpoint_iter_last >= my_bb_thread_params->checkpoint_iter_inc) {
	    std::chrono::high_resolution_clock::time_point t_now = std::chrono::high_resolution_clock::now();
	    long ms = std::chrono::duration_cast<std::chrono::milliseconds>(t_now - my_bb_thread_params->t_start).count();
	    if (save_checkpoint(my_bb_thread_params->checkpoint_filename, my_bb_thread_params, boxqueue, ms)) {
		my_bb_thread_params->checkpoint_iter_last = my_bb_thread_params->iterations;
	    }
	}

        //uncomment for verbose output
        //fprintf(stdout,"%12lu (%6d, %12lu): %18.15lf %18.15lf %18.15lf\n",iter,currentbox.depth,boxqueue.size(),CGAL::to_double(a),CGAL::to_double(currentbox.upper_bound_on_max_in_box),CGAL::to_double(bestyet));
        
        //split the box into two descendents
        j = index_of_coordinate_to_split(currentbox,*my_bb_thread_params);
        newbox[0] = currentbox;
        newbox[1] = currentbox;
        split_interval(currentbox.coord_bound_intervals[j],&(newbox[0].coord_bound_intervals[j]),&(newbox[1].coord_bound_intervals[j]));
	//calculate the upper bound for the descendents
        for (i=0;i<2;i++) {
            newbound[i] = area_of_union(newbox[i],*my_bb_thread_params);
        }

        for (i=0;i<2;i++) {
	    //if descendent cannot be discarded
            if ( (newbound[i] < 0) || (newbound[i] > bestyet) ) {
		//update its depth and upper bound and push it to the queue
                newbox[i].depth++;
                newbox[i].upper_bound_on_max_in_box = newbound[i];
                boxqueue.push(newbox[i]);
            }
	    //if it can be discarded, just do nothing
        }

    }

    //this point should only be reach if the initial "lower bound" was greater than the actual supremum,
    //otherwise the loop should continue ad infinitum giving progressively better and better bounds.
    std::cerr << "stopped after " << my_bb_thread_params->iterations << " iterations" << std::endl;

    return 0;
}

bool save_checkpoint(const std::string &filename, struct bb_thread_params *params, const std::priority_queue<struct box, std::vector<struct box>, struct CompareBoxes> &boxqueue, long elapsed_ms) {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open checkpoint file '" << filename << "' for writing." << std::endl;
        return false;
    }

    // Write header
    outfile << "# SofaBounds checkpoint file" << std::endl;
    outfile << "# Do not edit manually" << std::endl;
    outfile << std::endl;

    // Save problem specification
    outfile << params->num_intermediate << std::endl;
    for (unsigned int i = 0; i < params->num_intermediate; i++) {
        outfile << params->intermediate[i].a << " " << params->intermediate[i].b << " " << params->intermediate[i].c << std::endl;
    }
    outfile << params->initial.a << " " << params->initial.b << " " << params->initial.c << std::endl;
    outfile << params->final_min.a << " " << params->final_min.b << " " << params->final_min.c << std::endl;
    outfile << params->final_max.a << " " << params->final_max.b << " " << params->final_max.c << std::endl;
    outfile << params->has_final << std::endl;

    // Save runtime state
    outfile << rat_str(params->lower_bound) << std::endl;
    outfile << rat_str(params->upper_bound) << std::endl;
    outfile << params->iterations << std::endl;
    outfile << elapsed_ms << std::endl;

    // Save lower bound witness
    outfile << params->lower_bound_witness.size() << std::endl;
    for (unsigned int i = 0; i < params->lower_bound_witness.size(); i++) {
        outfile << rat_str(params->lower_bound_witness[i]) << std::endl;
    }

    // Save lower bound polygon
    outfile << params->lower_bound_polygon.size() << std::endl;
    for (unsigned int i = 0; i < params->lower_bound_polygon.size(); i++) {
        outfile << rat_str(params->lower_bound_polygon[i]) << std::endl;
    }

    // Save upper bound polygon
    outfile << params->upper_bound_polygon.size() << std::endl;
    for (unsigned int i = 0; i < params->upper_bound_polygon.size(); i++) {
        outfile << rat_str(params->upper_bound_polygon[i]) << std::endl;
    }

    // Save priority queue - convert to vector for serialization
    std::vector<struct box> boxes_vector;
    std::priority_queue<struct box, std::vector<struct box>, struct CompareBoxes> temp_queue = boxqueue;
    while (!temp_queue.empty()) {
        boxes_vector.push_back(temp_queue.top());
        temp_queue.pop();
    }

    outfile << boxes_vector.size() << std::endl;
    for (unsigned int i = 0; i < boxes_vector.size(); i++) {
        outfile << boxes_vector[i].depth << std::endl;
        outfile << rat_str(boxes_vector[i].upper_bound_on_max_in_box) << std::endl;
        outfile << boxes_vector[i].coord_bound_intervals.size() << std::endl;
        for (unsigned int j = 0; j < boxes_vector[i].coord_bound_intervals.size(); j++) {
            outfile << rat_str(boxes_vector[i].coord_bound_intervals[j].left) << " ";
            outfile << rat_str(boxes_vector[i].coord_bound_intervals[j].right) << std::endl;
        }
    }

    outfile.close();
    return true;
}

bool load_checkpoint(const std::string &filename, struct bb_thread_params *params, std::priority_queue<struct box, std::vector<struct box>, struct CompareBoxes> &boxqueue, long &elapsed_ms) {
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        return false;
    }

    std::string line;
    // Skip header comments
    while (std::getline(infile, line)) {
        if (line.empty() || line[0] == '#') continue;
        break;
    }

    // Load problem specification
    std::stringstream ss(line);
    ss >> params->num_intermediate;

    params->intermediate.resize(params->num_intermediate);
    for (unsigned int i = 0; i < params->num_intermediate; i++) {
        infile >> params->intermediate[i].a >> params->intermediate[i].b >> params->intermediate[i].c;
    }
    infile >> params->initial.a >> params->initial.b >> params->initial.c;
    infile >> params->final_min.a >> params->final_min.b >> params->final_min.c;
    infile >> params->final_max.a >> params->final_max.b >> params->final_max.c;
    infile >> params->has_final;

    // Load runtime state
    std::string lower_str, upper_str;
    infile >> lower_str;
    params->lower_bound = ExactRational(lower_str.c_str());
    infile >> upper_str;
    params->upper_bound = ExactRational(upper_str.c_str());
    infile >> params->iterations;
    infile >> elapsed_ms;

    // Load lower bound witness
    size_t witness_size;
    infile >> witness_size;
    params->lower_bound_witness.resize(witness_size);
    for (unsigned int i = 0; i < witness_size; i++) {
        std::string val;
        infile >> val;
        params->lower_bound_witness[i] = ExactRational(val.c_str());
    }

    // Load lower bound polygon
    size_t poly_size;
    infile >> poly_size;
    params->lower_bound_polygon.resize(poly_size);
    for (unsigned int i = 0; i < poly_size; i++) {
        std::string val;
        infile >> val;
        params->lower_bound_polygon[i] = ExactRational(val.c_str());
    }

    // Load upper bound polygon
    infile >> poly_size;
    params->upper_bound_polygon.resize(poly_size);
    for (unsigned int i = 0; i < poly_size; i++) {
        std::string val;
        infile >> val;
        params->upper_bound_polygon[i] = ExactRational(val.c_str());
    }

    // Load priority queue
    size_t queue_size;
    infile >> queue_size;
    
    // Clear existing queue
    while (!boxqueue.empty()) {
        boxqueue.pop();
    }

    for (unsigned int i = 0; i < queue_size; i++) {
        struct box b;
        infile >> b.depth;
        std::string ub_str;
        infile >> ub_str;
        b.upper_bound_on_max_in_box = ExactRational(ub_str.c_str());
        
        size_t intervals_size;
        infile >> intervals_size;
        b.coord_bound_intervals.resize(intervals_size);
        
        for (unsigned int j = 0; j < intervals_size; j++) {
            std::string left_str, right_str;
            infile >> left_str >> right_str;
            b.coord_bound_intervals[j].left = ExactRational(left_str.c_str());
            b.coord_bound_intervals[j].right = ExactRational(right_str.c_str());
        }
        
        boxqueue.push(b);
    }

    infile.close();
    return true;
}

