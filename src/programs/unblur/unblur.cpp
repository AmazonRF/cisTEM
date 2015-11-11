#include "../../core/core_headers.h"

class
UnBlurApp : public MyApp
{

	public:

	bool DoCalculation();
	void DoInteractiveUserInput();

	private:

};

void unblur_refine_alignment(Image *input_stack, int number_of_images, int max_iterations, float unitless_bfactor, bool mask_central_cross, int width_of_vertical_line, int width_of_horizontal_line, float inner_radius_for_peak_search, float outer_radius_for_peak_search, float max_shift_convergence_threshold, float pixel_size, float *x_shifts, float *y_shifts);

IMPLEMENT_APP(UnBlurApp)

// override the DoInteractiveUserInput

void UnBlurApp::DoInteractiveUserInput()
{
	std::string input_filename;
	std::string output_filename;
	float original_pixel_size;
	float minimum_shift_in_angstroms;
	float maximum_shift_in_angstroms;
	bool should_dose_filter = true;
	bool should_restore_power = true;
	float termination_threshold_in_angstroms;
	int max_iterations;
	float bfactor_in_angstroms;
	bool should_mask_central_cross = true;
	int horizontal_mask_size;
	int vertical_mask_size;
	float exposure_per_frame;
	float acceleration_voltage;
	float pre_exposure_amount;

	bool set_expert_options;

	 UserInput *my_input = new UserInput("Unblur", 1.0);

	 input_filename = my_input->GetFilenameFromUser("Input stack filename", "The input file, containing your raw movie frames", "my_movie.mrc", true );
	 output_filename = my_input->GetFilenameFromUser("Output aligned sum", "The output file, containing a weighted sum of the aligned input frames", "my_aligned_sum.mrc", false);
	 original_pixel_size = my_input->GetFloatFromUser("Pixel size of images (A)", "Pixel size of input images in Angstroms", "1.0", 0.0);
	 should_dose_filter = my_input->GetYesNoFromUser("Apply Exposure filter?", "Apply an exposure-dependent filter to frames before summing them", "YES");

	 if (should_dose_filter == true)
	 {
		 exposure_per_frame = my_input->GetFloatFromUser("Exposure per frame (e/A^2)", "Exposure per frame, in electrons per square Angstrom", "1.0", 0.0);
		 acceleration_voltage = my_input->GetFloatFromUser("Acceleration voltage (kV)", "Acceleration voltage during imaging", "300.0");
	 	 pre_exposure_amount = my_input->GetFloatFromUser("Pre-exposure amount (e/A^2)", "Amount of pre-exposure prior to the first frame, in electrons per square Angstrom", "0.0", 0.0);
	 }
	 else
	 {
	 	 exposure_per_frame = 0.0;
	 	 acceleration_voltage = 0.0;
	 	 pre_exposure_amount = 0.0;
	 }

	 set_expert_options = my_input->GetYesNoFromUser("Set Expert Options?", "Set these for more control, hopefully not needed", "NO");

	 if (set_expert_options == true)
	 {
	 	 minimum_shift_in_angstroms = my_input->GetFloatFromUser("Minimum shift for initial search (A)", "Initial search will be limited to between the inner and outer radii.", "2.0", 0.0);
	 	 maximum_shift_in_angstroms = my_input->GetFloatFromUser("Outer radius shift limit (A)", "The maximum shift of each alignment step will be limited to this value.", "80.0", minimum_shift_in_angstroms);
	 	 bfactor_in_angstroms = my_input->GetFloatFromUser("B-factor to apply to images (A^2)", "This B-Factor will be used to filter the reference prior to alignment", "1500", 0.0);
	 	 vertical_mask_size = my_input->GetIntFromUser("Half-width of vertical Fourier mask", "The vertical line mask will be twice this size. The central cross mask helps\nreduce problems by line artefacts from the detector", "1", 1);
	 	 horizontal_mask_size = my_input->GetIntFromUser("Half-width of horizontal Fourier mask", "The horizontal line mask will be twice this size. The central cross mask helps\nreduce problems by line artefacts from the detector", "1", 1);
	 	 termination_threshold_in_angstroms = my_input->GetFloatFromUser("Termination shift threshold (A)", "Alignment will iterate until the maximum shift is below this value", "1", 0.0);
	 	 max_iterations = my_input->GetIntFromUser("Maximum number of iterations", "Alignment will stop at this number, even if the threshold shift is not reached", "20", 0);

	 	 if (should_dose_filter == true)
	 	 {
	 		 should_restore_power = my_input->GetYesNoFromUser("Restore Noise Power?", "Restore the power of the noise to the level it would be without exposure filtering", "YES");
	 	 }
 	 }
 	 else
 	 {
 		 minimum_shift_in_angstroms = original_pixel_size + 0.001;
 		 maximum_shift_in_angstroms = 100.0;
 		 bfactor_in_angstroms = 1500.0;
 		 vertical_mask_size = 1;
 		 horizontal_mask_size = 1;
 		 termination_threshold_in_angstroms = original_pixel_size / 2;
 		 max_iterations = 20;
 		 should_restore_power = true;
 	 }

	 delete my_input;

	 my_current_job.Reset(13);
	 my_current_job.ManualSetArguments("ttfffbbfifbii",  input_filename.c_str(),
			 	 	 	 	 	 	 	 	 	 	 	 output_filename.c_str(),
														 original_pixel_size,
														 minimum_shift_in_angstroms,
														 maximum_shift_in_angstroms,
														 should_dose_filter,
														 should_restore_power,
														 termination_threshold_in_angstroms,
														 max_iterations,
														 bfactor_in_angstroms,
														 should_mask_central_cross,
														 horizontal_mask_size,
														 vertical_mask_size);


}

// overide the do calculation method which will be what is actually run..

bool UnBlurApp::DoCalculation()
{
	int pre_binning_factor;
	long image_counter;

	float unitless_bfactor;

	float pixel_size;
	float min_shift_in_pixels;
	float max_shift_in_pixels;
	float termination_threshold_in_pixels;

	Image sum_image;

	// get the arguments for this job..

	std::string input_filename 						= my_current_job.arguments[0].ReturnStringArgument();
	std::string output_filename 					= my_current_job.arguments[1].ReturnStringArgument();
	float       original_pixel_size					= my_current_job.arguments[2].ReturnFloatArgument();
	float 		minumum_shift_in_angstroms			= my_current_job.arguments[3].ReturnFloatArgument();
	float 		maximum_shift_in_angstroms			= my_current_job.arguments[4].ReturnFloatArgument();
	bool 		should_dose_filter					= my_current_job.arguments[5].ReturnBoolArgument();
	bool        should_restore_power				= my_current_job.arguments[6].ReturnBoolArgument();
	float 		termination_threshold_in_angstoms	= my_current_job.arguments[7].ReturnFloatArgument();
	int         max_iterations						= my_current_job.arguments[8].ReturnIntegerArgument();
	float 		bfactor_in_angstoms					= my_current_job.arguments[9].ReturnFloatArgument();
	bool        should_mask_central_cross			= my_current_job.arguments[10].ReturnBoolArgument();
	int         horizontal_mask_size				= my_current_job.arguments[11].ReturnIntegerArgument();
	int         vertical_mask_size					= my_current_job.arguments[12].ReturnIntegerArgument();

	//my_current_job.PrintAllArguments();

	// The Files

	MRCFile input_file(input_filename, false);
	MRCFile output_file(output_filename, true);

	long number_of_input_images = input_file.ReturnNumberOfSlices();

	// Arrays to hold the input images

	Image *unbinned_image_stack; // We will allocate this later depending on if we are binning or not.
	Image *image_stack = new Image[number_of_input_images];

	// Arrays to hold the shifts..

	float *x_shifts = new float[number_of_input_images];
	float *y_shifts = new float[number_of_input_images];

	// some quick checks..

	if (number_of_input_images <= 2)
	{
		SendError(wxString::Format("Error: Movie (%s) contains less than 3 frames.. Terminating.", input_filename));
		ExitMainLoop();
	}

	// Read in and FFT all the images..

	for (image_counter = 0; image_counter < number_of_input_images; image_counter++)
	{
		image_stack[image_counter].ReadSlice(&input_file, image_counter + 1);
		image_stack[image_counter].ForwardFFT(true);

		x_shifts[image_counter] = 0.0;
		y_shifts[image_counter] = 0.0;

	}

	// if we are binning - choose a binning factor..

	pre_binning_factor = int(myround(5. / original_pixel_size));
	if (pre_binning_factor < 1) pre_binning_factor = 1;

	wxPrintf("Prebinning factor = %i\n", pre_binning_factor);

	// if we are going to be binning, we need to allocate the unbinned array..

	if (pre_binning_factor > 1)
	{
		unbinned_image_stack = new Image[number_of_input_images];
		pixel_size = original_pixel_size * pre_binning_factor;
	}
	else
	{
		pixel_size = original_pixel_size;
	}

	// convert shifts to pixels..

	min_shift_in_pixels = minumum_shift_in_angstroms / pixel_size;
	max_shift_in_pixels = maximum_shift_in_angstroms / pixel_size;
	termination_threshold_in_pixels = termination_threshold_in_angstoms / pixel_size;


	// calculate the bfactor

	unitless_bfactor = bfactor_in_angstoms / pow(pixel_size, 2);

	if (min_shift_in_pixels <= 1.01) min_shift_in_pixels = 1.01;  // we always want to ignore the central peak initially.

	if (termination_threshold_in_pixels < 1 && pre_binning_factor > 1) termination_threshold_in_pixels = 1;

	if (pre_binning_factor > 1)
	{
		for (image_counter = 0; image_counter < number_of_input_images; image_counter++)
		{
			unbinned_image_stack[image_counter] = image_stack[image_counter];
			image_stack[image_counter].Resize(unbinned_image_stack[image_counter].logical_x_dimension / pre_binning_factor, unbinned_image_stack[image_counter].logical_y_dimension / pre_binning_factor, 1);
			//image_stack[image_counter].QuickAndDirtyWriteSlice("binned.mrc", image_counter + 1);
		}
	}

	// do the initial refinement (only 1 round - with the min shift)

	unblur_refine_alignment(image_stack, number_of_input_images, 1, unitless_bfactor, should_mask_central_cross, vertical_mask_size, horizontal_mask_size, min_shift_in_pixels, max_shift_in_pixels, termination_threshold_in_pixels, pixel_size, x_shifts, y_shifts);

	// now do the actual refinement..

	unblur_refine_alignment(image_stack, number_of_input_images, max_iterations, unitless_bfactor, should_mask_central_cross, vertical_mask_size, horizontal_mask_size, 0., max_shift_in_pixels, termination_threshold_in_pixels, pixel_size, x_shifts, y_shifts);


	// if we have been using pre-binning, we need to do a refinment on the unbinned data..

	if (pre_binning_factor > 1)
	{
		// we don't need the binned images anymore..

		delete [] image_stack;
		image_stack = unbinned_image_stack;
		pixel_size = original_pixel_size;

		// Adjust the shifts, then phase shift the original images

		for (image_counter = 0; image_counter < number_of_input_images; image_counter++)
		{
			x_shifts[image_counter] *= pre_binning_factor;
			y_shifts[image_counter] *= pre_binning_factor;

			image_stack[image_counter].PhaseShift(x_shifts[image_counter], y_shifts[image_counter], 0.0);
		}

		// convert parameters to pixels with new pixel size..

		min_shift_in_pixels = minumum_shift_in_angstroms / original_pixel_size;
		max_shift_in_pixels = maximum_shift_in_angstroms / original_pixel_size;
		termination_threshold_in_pixels = termination_threshold_in_angstoms / original_pixel_size;

		// recalculate the bfactor

		unitless_bfactor = bfactor_in_angstoms / pow(original_pixel_size, 2);

		// do the refinement..

		unblur_refine_alignment(image_stack, number_of_input_images, max_iterations, unitless_bfactor, should_mask_central_cross, vertical_mask_size, horizontal_mask_size, 0., max_shift_in_pixels, termination_threshold_in_pixels, original_pixel_size, x_shifts, y_shifts);

		// if allocated delete the binned stack, and swap the unbinned to image_stack - so that no matter what is happening we can just use image_stack



	}

	// we should be finished with alignment, now we just need to make the final sum..

	/*
    ! Dose filtering
    if (apply_dose_filter%value) then
        do image_counter = 1,number_of_frames_per_movie%value
            call my_electron_dose%ApplyDoseFilterToImage(image_stack(image_counter), &
                                                         dose_start=((image_counter-1)*exposure_per_frame%value) &
                                                                    + pre_exposure_amount%value, &
                                                         dose_finish=(image_counter*exposure_per_frame%value) &
                                                                    + pre_exposure_amount%value, &
                                                         pixel_size=pixel_size%value)
        enddo
    endif

    */


	sum_image.Allocate(image_stack[0].logical_x_dimension, image_stack[0].logical_y_dimension, false);
	sum_image.SetToConstant(0.0);

	for (image_counter = 0; image_counter < number_of_input_images; image_counter++)
	{
		sum_image.AddImage(&image_stack[image_counter]);
		wxPrintf("#%li = %f, %f\n", image_counter, x_shifts[image_counter] * pixel_size, y_shifts[image_counter] * pixel_size);
	}

	// now we just need to write out the final sum..

	sum_image.WriteSlice(&output_file, 1);

	delete [] x_shifts;
	delete [] y_shifts;
	delete [] image_stack;

	return true;
}

void unblur_refine_alignment(Image *input_stack, int number_of_images, int max_iterations, float unitless_bfactor, bool mask_central_cross, int width_of_vertical_line, int width_of_horizontal_line, float inner_radius_for_peak_search, float outer_radius_for_peak_search, float max_shift_convergence_threshold, float pixel_size, float *x_shifts, float *y_shifts)
{
	long pixel_counter;
	long image_counter;
	long iteration_counter;

	int number_of_middle_image = number_of_images / 2;

	float *current_x_shifts = new float[number_of_images];
	float *current_y_shifts = new float[number_of_images];

	float middle_image_x_shift;
	float middle_image_y_shift;

	float max_shift;
	float total_shift;

	Image sum_of_images;
	Image sum_of_images_minus_current;

	Peak my_peak;

	Curve x_shifts_curve;
	Curve y_shifts_curve;

	sum_of_images.Allocate(input_stack[0].logical_x_dimension, input_stack[0].logical_y_dimension, false);
	sum_of_images.SetToConstant(0.0);

	sum_of_images_minus_current.Allocate(input_stack[0].logical_x_dimension, input_stack[0].logical_y_dimension, false);



	// prepare the initial sum

	for (image_counter = 0; image_counter < number_of_images; image_counter++)
	{
		sum_of_images.AddImage(&input_stack[image_counter]);
		current_x_shifts[image_counter] = 0;
		current_y_shifts[image_counter] = 0;
	}

	// perform the main alignment loop until we reach a max shift less than wanted, or max iterations

	for (iteration_counter = 1; iteration_counter <= max_iterations; iteration_counter++)
	{
		wxPrintf("Starting iteration number %li\n\n", iteration_counter);
		max_shift = -FLT_MAX;

		for (image_counter = 0; image_counter < number_of_images; image_counter++)
		{
			// prepare the sum reference by subtracting out the current image, applying a bfactor and masking central cross

			sum_of_images_minus_current = sum_of_images;
			sum_of_images_minus_current.SubtractImage(&input_stack[image_counter]);
			sum_of_images_minus_current.ApplyBFactor(unitless_bfactor);

			if (mask_central_cross == true)
			{
				sum_of_images_minus_current.MaskCentralCross(width_of_vertical_line, width_of_horizontal_line);
			}

			// compute the cross correlation function and find the peak

		    sum_of_images_minus_current.CalculateCrossCorrelationImageWith(&input_stack[image_counter]);
		    my_peak = sum_of_images_minus_current.FindPeakWithParabolaFit(inner_radius_for_peak_search, outer_radius_for_peak_search);

			// update the shifts..

			current_x_shifts[image_counter] = my_peak.x;
			current_y_shifts[image_counter] = my_peak.y;
		}

		// smooth the shifts

		x_shifts_curve.ClearData();
		y_shifts_curve.ClearData();

		for (image_counter = 0; image_counter < number_of_images; image_counter++)
		{
			x_shifts_curve.AddPoint(image_counter, x_shifts[image_counter] + current_x_shifts[image_counter]);
			y_shifts_curve.AddPoint(image_counter, y_shifts[image_counter] + current_y_shifts[image_counter]);

			//wxPrintf("Before = %li : %f, %f\n", image_counter, x_shifts[image_counter] + current_x_shifts[image_counter], y_shifts[image_counter] + current_y_shifts[image_counter]);
		}


		x_shifts_curve.FitSavitzkyGolayToData(5, 3);
		y_shifts_curve.FitSavitzkyGolayToData(5, 3);

		// copy them back..

		for (image_counter = 0; image_counter < number_of_images; image_counter++)
		{
			current_x_shifts[image_counter] = x_shifts_curve.savitzky_golay_fit[image_counter] - x_shifts[image_counter];
			current_y_shifts[image_counter] = y_shifts_curve.savitzky_golay_fit[image_counter] - y_shifts[image_counter];
			wxPrintf("After = %li : %f, %f\n", image_counter, x_shifts_curve.savitzky_golay_fit[image_counter], y_shifts_curve.savitzky_golay_fit[image_counter]);
		}



		// subtract shift of the middle image from all images to keep things centred around it

		middle_image_x_shift = current_x_shifts[number_of_middle_image];
		middle_image_y_shift = current_y_shifts[number_of_middle_image];

		for (image_counter = 0; image_counter < number_of_images; image_counter++)
		{
			current_x_shifts[image_counter] -= middle_image_x_shift;
			current_y_shifts[image_counter] -= middle_image_y_shift;

			total_shift = sqrt(pow(current_x_shifts[image_counter], 2) + pow(current_y_shifts[image_counter], 2));
			if (total_shift > max_shift) max_shift = total_shift;

		}

		// actually shift the images, also add the subtracted shifts to the overall shifts

		for (image_counter = 0; image_counter < number_of_images; image_counter++)
		{
			input_stack[image_counter].PhaseShift(current_x_shifts[image_counter], current_y_shifts[image_counter], 0.0);

			x_shifts[image_counter] += current_x_shifts[image_counter];
			y_shifts[image_counter] += current_y_shifts[image_counter];
		}

		// check to see if the convergence criteria have been reached and return if so

		if (iteration_counter >= max_iterations || max_shift <= max_shift_convergence_threshold)
		{
			wxPrintf("returning, iteration = %li, max_shift = %f\n", iteration_counter, max_shift);
			delete [] current_x_shifts;
			delete [] current_y_shifts;
			return;
		}
		else
		{
			wxPrintf("Not. returning, iteration = %li, max_shift = %f\n", iteration_counter, max_shift);

		}

		// going to be doing another round so we need to make the new sum..

		sum_of_images.SetToConstant(0.0);

		for (image_counter = 0; image_counter < number_of_images; image_counter++)
		{
			sum_of_images.AddImage(&input_stack[image_counter]);
		}

	}

	delete [] current_x_shifts;
	delete [] current_y_shifts;

}




