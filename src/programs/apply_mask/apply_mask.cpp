#include "../../core/core_headers.h"

class
ApplyMask : public MyApp
{

	public:

	bool DoCalculation();
	void DoInteractiveUserInput();

	private:
};


IMPLEMENT_APP(ApplyMask)

// override the DoInteractiveUserInput

void ApplyMask::DoInteractiveUserInput()
{

	UserInput *my_input = new UserInput("ApplyMask", 1.0);

	wxString input_volume	= my_input->GetFilenameFromUser("Input image/volume file name", "Name of input image file", "input.mrc", true );
	wxString input_mask		= my_input->GetFilenameFromUser("Input mask file name", "Name of input image file", "mask.mrc", true );
	wxString output_volume	= my_input->GetFilenameFromUser("Output masked image/volume file name", "Name of output image with mask applied", "output.mrc", false );
	float pixel_size		= my_input->GetFloatFromUser("Pixel size of images (A)", "Pixel size of input images in Angstroms", "1.0", 0.000001);
	float cosine_edge		= my_input->GetFloatFromUser("Width of cosine edge (A)", "Width of the smooth edge to add to the mask in Angstroms", "10.0", 0.0);
	float outside_weight	= my_input->GetFloatFromUser("Weight of density outside mask", "Factor to multiply density outside of the mask", "0.0", 0.0, 1.0);
	float filter_radius		= my_input->GetFloatFromUser("Low-pass filter outside mask (A)", "Low-pass filter to be applied to the density outside the mask", "0.0", 0.0);

	delete my_input;

	my_current_job.Reset(7);
	my_current_job.ManualSetArguments("tttffff", input_volume.ToUTF8().data(), input_mask.ToUTF8().data(), output_volume.ToUTF8().data(), pixel_size, cosine_edge, outside_weight, filter_radius);
}

// override the do calculation method which will be what is actually run..

bool ApplyMask::DoCalculation()
{

	wxString input_volume	= my_current_job.arguments[0].ReturnStringArgument();
	wxString input_mask		= my_current_job.arguments[1].ReturnStringArgument();
	wxString output_volume	= my_current_job.arguments[2].ReturnStringArgument();
	float pixel_size		= my_current_job.arguments[3].ReturnFloatArgument();
	float cosine_edge		= my_current_job.arguments[4].ReturnFloatArgument();
	float outside_weight	= my_current_job.arguments[5].ReturnFloatArgument();
	float filter_radius		= my_current_job.arguments[6].ReturnFloatArgument();

	MRCFile input3d_file(input_volume.ToStdString(), false);
	MRCFile input_mask_file(input_mask.ToStdString(), false);
	MRCFile output_file(output_volume.ToStdString(), true);

	float filter_edge = 10.0;
	float mask_volume;
	Image my_image;
	Image my_mask;

//	if (input3d_file.ReturnZSize() > 1)
//	{
		wxPrintf("\nMasking Image/Volume...\n");
		my_image.ReadSlices(&input3d_file, 1, input3d_file.ReturnNumberOfSlices());
		my_mask.ReadSlices(&input_mask_file, 1, input_mask_file.ReturnNumberOfSlices());
		if (! my_image.HasSameDimensionsAs(&my_mask))
		{
			wxPrintf("\nImage/Volume and mask file have different dimensions\n");
			exit(0);
		}
/*		if (! my_image.IsCubic())
		{
			wxPrintf("\nVolume is not cubic\n");
			exit(0);
		} */
		mask_volume = my_image.ApplyMask(my_mask, cosine_edge / pixel_size, outside_weight, pixel_size / filter_radius, pixel_size / filter_edge);
		my_image.WriteSlices(&output_file,1, input3d_file.ReturnNumberOfSlices());
/*	}
	else
	{
		wxPrintf("\nMasking Images...\n\n");
		my_image.ReadSlice(&input3d_file, 1);
		my_mask.ReadSlice(&input_mask_file, 1);
		if (! my_image.HasSameDimensionsAs(&my_mask))
		{
			wxPrintf("\nImage and mask file have different dimensions\n");
			exit(0);
		}
		if (! my_image.IsSquare())
		{
			wxPrintf("\nImage is not square\n");
			exit(0);
		}
		my_image.ApplyMask(my_mask, cosine_edge / pixel_size, outside_weight, pixel_size / filter_radius, pixel_size / filter_edge);
		my_image.WriteSlices(&output_file,1, input3d_file.ReturnNumberOfSlices());
	} */

	wxPrintf("\nMask volume = %g\n\n", mask_volume);

	return true;
}
