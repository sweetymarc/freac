 /* BonkEnc Audio Encoder
  * Copyright (C) 2001-2006 Robert Kausch <robert.kausch@bonkenc.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the "GNU General Public License".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#include <smooth/main.h>
#include <smooth/args.h>
#include <cmdmain.h>
#include <joblist.h>

using namespace smooth::System;

Int smooth::Main(const Array<String> &args)
{
	BonkEnc::debug_out = new BonkEnc::Debug("BonkEnc.log");

	BonkEnc::debug_out->OutputLine("");
	BonkEnc::debug_out->OutputLine("=========================================");
	BonkEnc::debug_out->OutputLine("= Starting BonkEnc command line tool... =");
	BonkEnc::debug_out->OutputLine("=========================================");
	BonkEnc::debug_out->OutputLine("");

	BonkEnc::BonkEncCommandline	*app = new BonkEnc::BonkEncCommandline(args);

	delete app;

	BonkEnc::debug_out->OutputLine("");
	BonkEnc::debug_out->OutputLine("======================================");
	BonkEnc::debug_out->OutputLine("= Leaving BonkEnc command line tool! =");
	BonkEnc::debug_out->OutputLine("======================================");

	delete BonkEnc::debug_out;

	return 0;
}

BonkEnc::BonkEncCommandline::BonkEncCommandline(const Array<String> &arguments) : args(arguments)
{
	currentConfig->enable_console = true;
	currentConfig->appMain = this;

	if (currentConfig->language == "") i18n->ActivateLanguage("internal");

	i18n->ActivateLanguage(currentConfig->language);

	joblist	= new JobList(Point(0, 0), Size(0, 0));

	bool		 quiet		= ScanForParameter("-quiet", NULL);
	Array<String>	 files;
	String		 encoder	= "FAAC";
	String		 helpenc	= "";
	String		 outdir		= ".";
	String		 outfile	= "";

	ScanForParameter("-e", &encoder);
	ScanForParameter("-h", &helpenc);
	ScanForParameter("-d", &outdir);
	ScanForParameter("-o", &outfile);

	ScanForFiles(&files);

	Console::SetTitle(String("BonkEnc ").Append(BonkEnc::version));

	if ((files.GetNOfEntries() == 0 && helpenc == "") || !(encoder == "LAME" || encoder == "VORBIS" || encoder == "BONK" || encoder == "BLADE" || encoder == "FAAC" || encoder == "FLAC" || encoder == "TVQ" || encoder == "WAVE" || encoder == "lame" || encoder == "vorbis" || encoder == "bonk" || encoder == "blade" || encoder == "faac" || encoder == "flac" || encoder == "tvq" || encoder == "wave") || (files.GetNOfEntries() > 1 && outfile != ""))
	{
		Console::OutputString(String("BonkEnc Audio Encoder ").Append(BonkEnc::version).Append(" command line interface\nCopyright (C) 2001-2006 Robert Kausch\n\n"));
		Console::OutputString("Usage:\tBEcmd [options] [file(s)]\n\n");
		Console::OutputString("\t-e <encoder>\tSpecify the encoder to use (default is FAAC)\n");
		Console::OutputString("\t-d <outdir>\tSpecify output directory for encoded files\n");
		Console::OutputString("\t-o <outfile>\tSpecify output file name in single file mode\n");
		Console::OutputString("\t-h <encoder>\tPrint help for encoder specific options\n");
		Console::OutputString("\t-quiet\t\tDo not print any messages\n\n");
		Console::OutputString("<encoder> can be one of LAME, VORBIS, BONK, BLADE, FAAC, FLAC, TVQ or WAVE.\n\n");
	}
	else if (helpenc != "")
	{
		Console::OutputString(String("BonkEnc Audio Encoder ").Append(BonkEnc::version).Append(" command line interface\nCopyright (C) 2001-2006 Robert Kausch\n\n"));

		if (helpenc == "LAME" || helpenc == "lame")
		{
			Console::OutputString("Options for LAME MP3 encoder:\n\n");
			Console::OutputString("\t-m <mode>\t\t(CBR, VBR or ABR, default: VBR)\n");
			Console::OutputString("\t-b <CBR/ABR bitrate>\t(8 - 320, default: 192)\n");
			Console::OutputString("\t-q <VBR quality>\t(0 = best, 9 = worst, default: 5)\n\n");
		}
		else if (helpenc == "VORBIS" || helpenc == "vorbis")
		{
			Console::OutputString("Options for Ogg Vorbis encoder:\n\n");
			Console::OutputString("\t-q <quality>\t\t(0 - 100, default: 60, VBR mode)\n");
			Console::OutputString("\t-b <target bitrate>\t(45 - 500, default: 192, ABR mode)\n\n");
		}
		else if (helpenc == "BONK" || helpenc == "bonk")
		{
			Console::OutputString("Options for Bonk encoder:\n\n");
			Console::OutputString("\t-q <quantization factor>\t(0 - 2, default: 0.4)\n");
			Console::OutputString("\t-p <predictor size>\t\t(0 - 512, default: 32)\n");
			Console::OutputString("\t-r <downsampling ratio>\t\t(1 - 10, default: 2)\n");
			Console::OutputString("\t-js\t\t\t\t(use Joint Stereo)\n");
			Console::OutputString("\t-lossless\t\t\t(use lossless compression)\n\n");
		}
		else if (helpenc == "BLADE" || helpenc == "blade")
		{
			Console::OutputString("Options for BladeEnc encoder:\n\n");
			Console::OutputString("\t-b <bitrate>\t(32, 40, 48, 56, 64, 80, 96, 112, 128,\n");
			Console::OutputString("\t\t\t 160, 192, 224, 256 or 320, default: 192)\n\n");
		}
		else if (helpenc == "FAAC" || helpenc == "faac")
		{
			Console::OutputString("Options for FAAC AAC/MP4 encoder:\n\n");
			Console::OutputString("\t-q <quality>\t\t\t(10 - 500, default: 100, VBR mode)\n");
			Console::OutputString("\t-b <bitrate per channel>\t(8 - 256, default: 64, ABR mode)\n");
			Console::OutputString("\t-mp4\t\t\t\t(use MP4 container format)\n\n");
		}
		else if (helpenc == "FLAC" || helpenc == "flac")
		{
			Console::OutputString("Options for FLAC encoder:\n\n");
			Console::OutputString("\t-b <blocksize>\t\t\t(192 - 32768, default: 4608)\n");
			Console::OutputString("\t-m\t\t\t\t(use mid-side stereo)\n");
			Console::OutputString("\t-e\t\t\t\t(do exhaustive model search)\n");
			Console::OutputString("\t-l <max LPC order>\t\t(0 - 32, default: 8)\n");
			Console::OutputString("\t-q <QLP coeff precision>\t(0 - 16, default: 0)\n");
			Console::OutputString("\t-p\t\t\t\t(do exhaustive QLP coeff optimization)\n");
			Console::OutputString("\t-r <min Rice>,<max Rice>\t(0 - 16, default: 3,3)\n\n");
		}
		else if (helpenc == "TVQ" || helpenc == "tvq")
		{
			Console::OutputString("Options for TwinVQ encoder:\n\n");
			Console::OutputString("\t-b <bitrate per channel>\t(24, 32 or 48, default: 48)\n");
			Console::OutputString("\t-c <preselection candidates>\t(4, 8, 16 or 32, default: 32)\n\n");
		}
		else if (helpenc == "WAVE" || helpenc == "wave")
		{
			Console::OutputString("No options can be configured for the WAVE Out filter!\n\n");
		}
		else
		{
			Console::OutputString(String("Encoder ").Append(helpenc).Append(" is not supported by BonkEnc!\n\n"));
		}
	}
	else
	{
		bool	 broken = false;

		if (((encoder == "LAME" || encoder == "lame")	  && !currentConfig->enable_lame)   ||
		    ((encoder == "VORBIS" || encoder == "vorbis") && !currentConfig->enable_vorbis) ||
		    ((encoder == "BONK" || encoder == "bonk")	  && !currentConfig->enable_bonk)   ||
		    ((encoder == "BLADE" || encoder == "blade")	  && !currentConfig->enable_blade)  ||
		    ((encoder == "FAAC" || encoder == "faac")	  && !currentConfig->enable_faac)   ||
		    ((encoder == "FLAC" || encoder == "flac")	  && !currentConfig->enable_flac)   ||
		    ((encoder == "TVQ" || encoder == "tvq")	  && !currentConfig->enable_tvq))
		{
			Console::OutputString(String("Encoder ").Append(encoder).Append(" is not available!\n\n"));

			broken = true;
		}

		if (encoder == "LAME" || encoder == "lame")
		{
			String	 bitrate = "192";
			String	 quality = "5";
			String	 mode	 = "VBR";

			ScanForParameter("-b", &bitrate);
			ScanForParameter("-q", &quality);
			ScanForParameter("-m", &mode);

			currentConfig->lame_preset = 0;
			currentConfig->lame_set_bitrate = True;

			currentConfig->lame_bitrate    = Math::Max(0, Math::Min(320, bitrate.ToInt()));
			currentConfig->lame_abrbitrate = Math::Max(0, Math::Min(320, bitrate.ToInt()));
			currentConfig->lame_vbrquality = Math::Max(0, Math::Min(9, quality.ToInt()));

			if (mode == "VBR" || mode == "vbr")	 currentConfig->lame_vbrmode = 2;
			else if (mode == "ABR" || mode == "abr") currentConfig->lame_vbrmode = 3;
			else if (mode == "CBR" || mode == "cbr") currentConfig->lame_vbrmode = 0;

			currentConfig->encoder = ENCODER_LAMEENC;
		}
		else if (encoder == "VORBIS" || encoder == "vorbis")
		{
			String	 bitrate = "192";
			String	 quality = "60";

			if (ScanForParameter("-b", &bitrate))		currentConfig->vorbis_mode = 1;
			else if (ScanForParameter("-q", &quality))	currentConfig->vorbis_mode = 0;
			else						currentConfig->vorbis_mode = 0;

			currentConfig->vorbis_quality = Math::Max(0, Math::Min(100, quality.ToInt()));
			currentConfig->vorbis_bitrate = Math::Max(45, Math::Min(500, bitrate.ToInt()));

			currentConfig->encoder = ENCODER_VORBISENC;
		}
		else if (encoder == "BONK" || encoder == "bonk")
		{
			String	 quantization = "0.4";
			String	 predictor    = "32";
			String	 downsampling = "2";

			ScanForParameter("-q", &quantization);
			ScanForParameter("-p", &predictor);
			ScanForParameter("-r", &downsampling);

			currentConfig->bonk_jstereo	 = ScanForParameter("-js", NULL);
			currentConfig->bonk_lossless	 = ScanForParameter("-lossless", NULL);

			currentConfig->bonk_quantization = Math::Max(0, Math::Min(40, Math::Round(quantization.ToFloat() * 20)));
			currentConfig->bonk_predictor	 = Math::Max(0, Math::Min(512, predictor.ToInt()));
			currentConfig->bonk_downsampling = Math::Max(0, Math::Min(10, downsampling.ToInt()));

			currentConfig->encoder = ENCODER_BONKENC;
		}
		else if (encoder == "BLADE" || encoder == "blade")
		{
			String	 bitrate = "192";

			ScanForParameter("-b", &bitrate);

			currentConfig->blade_bitrate = Math::Max(32, Math::Min(320, bitrate.ToInt()));

			currentConfig->encoder = ENCODER_BLADEENC;
		}
		else if (encoder == "FAAC" || encoder == "faac")
		{
			String	 bitrate = "64";
			String	 quality = "100";

			if (ScanForParameter("-b", &bitrate))		currentConfig->faac_set_quality = False;
			else if (ScanForParameter("-q", &quality))	currentConfig->faac_set_quality = True;
			else						currentConfig->faac_set_quality = True;

			currentConfig->faac_enable_mp4	= ScanForParameter("-mp4", NULL);

			currentConfig->faac_aac_quality	= Math::Max(10, Math::Min(500, quality.ToInt()));
			currentConfig->faac_bitrate	= Math::Max(8, Math::Min(256, bitrate.ToInt()));

			currentConfig->encoder = ENCODER_FAAC;
		}
		else if (encoder == "FLAC" || encoder == "flac")
		{
			String	 blocksize = "4608";
			String	 lpc = "8";
			String	 qlp = "0";
			String	 rice = "3,3";
			String	 minrice;
			String	 maxrice;

			ScanForParameter("-b", &blocksize);
			ScanForParameter("-l", &lpc);
			ScanForParameter("-q", &qlp);
			ScanForParameter("-r", &rice);

			Int	 i = 0;
			Int	 j = 0;

			for (i = 0; i < rice.Length(); i++)	{ if (rice[i] == ',') break; minrice[i] = rice[i]; }
			for (j = i + 1; j < rice.Length(); j++)	{ maxrice[j - i - 1] = rice[j]; }

			currentConfig->flac_preset = -1;

			currentConfig->flac_do_mid_side_stereo		 = ScanForParameter("-m", NULL);
			currentConfig->flac_do_exhaustive_model_search	 = ScanForParameter("-e", NULL);
			currentConfig->flac_do_qlp_coeff_prec_search	 = ScanForParameter("-p", NULL);

			currentConfig->flac_blocksize			 = Math::Max(192, Math::Min(32768, blocksize.ToInt()));
			currentConfig->flac_max_lpc_order		 = Math::Max(0, Math::Min(32, lpc.ToInt()));
			currentConfig->flac_qlp_coeff_precision		 = Math::Max(0, Math::Min(16, qlp.ToInt()));
			currentConfig->flac_min_residual_partition_order = Math::Max(0, Math::Min(16, minrice.ToInt()));
			currentConfig->flac_max_residual_partition_order = Math::Max(0, Math::Min(16, maxrice.ToInt()));

			currentConfig->encoder = ENCODER_FLAC;
		}
		else if (encoder == "TVQ" || encoder == "tvq")
		{
			String	 bitrate    = "48";
			String	 candidates = "32";

			ScanForParameter("-b", &bitrate);
			ScanForParameter("-c", &candidates);

			currentConfig->tvq_presel_candidates = Math::Max(4, Math::Min(32, candidates.ToInt()));
			currentConfig->tvq_bitrate	     = Math::Max(24, Math::Min(48, bitrate.ToInt()));

			currentConfig->encoder = ENCODER_TVQ;
		}
		else if (encoder == "WAVE" || encoder == "wave")
		{
			currentConfig->encoder = ENCODER_WAVE;
		}
		else
		{
			Console::OutputString(String("Encoder ").Append(encoder).Append(" is not supported by BonkEnc!\n\n"));

			broken = true;
		}

		if (!broken)
		{
			currentConfig->enc_outdir = outdir;

			if (currentConfig->enc_outdir[currentConfig->enc_outdir.Length() - 1] != '\\') currentConfig->enc_outdir.Append("\\");

			for (Int i = 0; i < files.GetNOfEntries(); i++)
			{
				InStream	*in = new InStream(STREAM_FILE, files.GetNthEntry(i), IS_READONLY);

				if (in->GetLastError() != IO_ERROR_OK)
				{
					delete in;

					Console::OutputString(String("File not found: ").Append(files.GetNthEntry(i)).Append("\n"));

					broken = true;

					continue;
				}
				else
				{
					delete in;
				}

				String	 extension;

				extension[0] = (files.GetNthEntry(i))[files.GetNthEntry(i).Length() - 4];
				extension[1] = (files.GetNthEntry(i))[files.GetNthEntry(i).Length() - 3];
				extension[2] = (files.GetNthEntry(i))[files.GetNthEntry(i).Length() - 2];
				extension[3] = (files.GetNthEntry(i))[files.GetNthEntry(i).Length() - 1];

				if ((extension == ".mp3" && !currentConfig->enable_lame) || (extension == ".ogg" && !currentConfig->enable_vorbis))
				{
					Console::OutputString(String("Cannot process file: ").Append(files.GetNthEntry(i)).Append("\n"));

					broken = true;

					continue;
				}

				if (!quiet) Console::OutputString(String("Processing file: ").Append(files.GetNthEntry(i)).Append("..."));

				joblist->AddTrackByFileName(files.GetNthEntry(i), outfile);

				Encode();

				while (encoding)
				{
					MSG	 msg;
					bool	 result;

					if (Setup::enableUnicode)	result = PeekMessageW(&msg, 0, 0, 0, PM_REMOVE);
					else				result = PeekMessageA(&msg, 0, 0, 0, PM_REMOVE);

					if (result)
					{
						TranslateMessage(&msg);

						if (Setup::enableUnicode)	DispatchMessageW(&msg);
						else				DispatchMessageA(&msg);
					}

					Sleep(10);
				}

				if (!quiet) Console::OutputString("done.\n");
			}
		}
	}

	delete joblist;
}

BonkEnc::BonkEncCommandline::~BonkEncCommandline()
{
}

Bool BonkEnc::BonkEncCommandline::ScanForParameter(const String &param, String *option)
{
	for (Int i = 0; i < args.GetNOfEntries(); i++)
	{
		if (args.GetNthEntry(i) == param)
		{
			if (option != NULL) *option = args.GetNthEntry(i + 1);

			return True;
		}
	}

	return False;
}

Void BonkEnc::BonkEncCommandline::ScanForFiles(Array<String> *files)
{
	String	 param;
	String	 prevParam;

	for (Int i = 0; i < args.GetNOfEntries(); i++)
	{
		prevParam	= param;
		param		= args.GetNthEntry(i);

		if (param[0] != '-' && (prevParam[0] != '-' || prevParam == "-q")) (*files).AddEntry(param);
	}
}
