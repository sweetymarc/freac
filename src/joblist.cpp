 /* BonkEnc Audio Encoder
  * Copyright (C) 2001-2005 Robert Kausch <robert.kausch@bonkenc.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the "GNU General Public License".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#include <joblist.h>
#include <main.h>
#include <dllinterfaces.h>
#include <utilities.h>
#include <input/filter-in-cdrip.h>

BonkEnc::JobList::JobList(Point iPos, Size iSize) : ListBox(iPos, iSize)
{
	onRegister.Connect(&JobList::OnRegister, this);
	onUnregister.Connect(&JobList::OnUnregister, this);

	droparea = new DropArea(iPos, iSize);
	droparea->onDropFile.Connect(&JobList::AddTrackByDragAndDrop, this);

	text = new Text(bonkEnc::i18n->TranslateString("%1 file(s) in joblist:").Replace("%1", "0"), iPos - Point(9, 19));

	button_sel_all		= new Button(NIL, Bitmap::LoadBitmap("BonkEnc.pci", 16, NIL), iPos - Point(19, 4), Size(21, 21));
	button_sel_all->onClick.Connect(&JobList::SelectAll, this);
	button_sel_all->SetFlags(BF_NOFRAME);
	button_sel_all->SetTooltipText(bonkEnc::i18n->TranslateString("Select all"));

	button_sel_none		= new Button(NIL, Bitmap::LoadBitmap("BonkEnc.pci", 17, NIL), iPos - Point(19, -10), Size(21, 21));
	button_sel_none->onClick.Connect(&JobList::SelectNone, this);
	button_sel_none->SetFlags(BF_NOFRAME);
	button_sel_none->SetTooltipText(bonkEnc::i18n->TranslateString("Select none"));

	button_sel_toggle	= new Button(NIL, Bitmap::LoadBitmap("BonkEnc.pci", 18, NIL), iPos - Point(19, -24), Size(21, 21));
	button_sel_toggle->onClick.Connect(&JobList::ToggleSelection, this);
	button_sel_toggle->SetFlags(BF_NOFRAME);
	button_sel_toggle->SetTooltipText(bonkEnc::i18n->TranslateString("Toggle selection"));
}

BonkEnc::JobList::~JobList()
{
	DeleteObject(droparea);
	DeleteObject(text);

	DeleteObject(button_sel_all);
	DeleteObject(button_sel_none);
	DeleteObject(button_sel_toggle);
}

Int BonkEnc::JobList::GetNOfTracks()
{
	return tracks.GetNOfEntries();
}

Track *BonkEnc::JobList::GetNthTrack(Int n)
{
	return tracks.GetNthEntry(n);
}

Bool BonkEnc::JobList::AddTrack(Track *track)
{
	String	 jlEntry;
	String	 tooltip;

	if (track->artist == NIL && track->title == NIL)	jlEntry = String(track->origFilename).Append("\t");
	else							jlEntry = String(track->artist.Length() > 0 ? track->artist : bonkEnc::i18n->TranslateString("unknown artist")).Append(" - ").Append(track->title.Length() > 0 ? track->title : bonkEnc::i18n->TranslateString("unknown title")).Append("\t");

	jlEntry.Append(track->track > 0 ? (track->track < 10 ? String("0").Append(String::FromInt(track->track)) : String::FromInt(track->track)) : String("")).Append("\t").Append(track->lengthString).Append("\t").Append(track->fileSizeString);

	tooltip = String(bonkEnc::i18n->TranslateString("File")).Append(": ").Append(track->origFilename).Append("\n").
		  Append(bonkEnc::i18n->TranslateString("Size")).Append(": ").Append(track->fileSizeString).Append(" ").Append(bonkEnc::i18n->TranslateString("bytes")).Append("\n").
		  Append(bonkEnc::i18n->TranslateString("Artist")).Append(": ").Append(track->artist.Length() > 0 ? track->artist : bonkEnc::i18n->TranslateString("unknown artist")).Append("\n").
		  Append(bonkEnc::i18n->TranslateString("Title")).Append(": ").Append(track->title.Length() > 0 ? track->title : bonkEnc::i18n->TranslateString("unknown title")).Append("\n").
		  Append(track->length > 0 ? bonkEnc::i18n->TranslateString("Length").Append(": ").Append(track->lengthString).Append(" ").Append(bonkEnc::i18n->TranslateString("min")).Append("\n") : "").
		  Append(track->length > 0 ? bonkEnc::i18n->TranslateString("Number of samples").Append(": ").Append(Utilities::LocalizeNumber(track->length)).Append("\n") : "").
		  Append(bonkEnc::i18n->TranslateString("Sampling rate")).Append(": ").Append(Utilities::LocalizeNumber(track->rate)).Append(" Hz\n").
		  Append(bonkEnc::i18n->TranslateString("Sample resolution")).Append(": ").Append(String::FromInt(track->bits)).Append(" ").Append(bonkEnc::i18n->TranslateString("bit")).Append("\n").
		  Append(bonkEnc::i18n->TranslateString("Channels")).Append(": ").Append((track->channels > 2 || track->channels < 1) ? String::FromInt(track->channels) : (track->channels == 1 ? bonkEnc::i18n->TranslateString("Mono") : bonkEnc::i18n->TranslateString("Stereo"))).
		  Append((track->length > 0 && track->rate > 0 && track->channels > 0) ? bonkEnc::i18n->TranslateString("\nBitrate").Append(": ").Append(String::FromInt((Int) Math::Round(((Float) track->fileSize) / (track->length / (track->rate * track->channels)) * 8.0 / 1024.0))).Append(" kbps") : "");

	ListEntry	*entry	= AddEntry(jlEntry);

	if (bonkEnc::currentConfig->showTooltips) entry->SetTooltipText(tooltip);

	entry->SetMark(True);

	tracks.AddEntry(track);

	return True;
}

Bool BonkEnc::JobList::RemoveNthTrack(Int n)
{
	delete GetNthTrack(n);

	tracks.RemoveEntry(n);

	RemoveEntry(GetNthEntry(n));

	text->SetText(bonkEnc::i18n->TranslateString("%1 file(s) in joblist:").Replace("%1", String::FromInt(GetNOfEntries())));

	return True;
}

Bool BonkEnc::JobList::RemoveAllTracks()
{
	if (bonkEnc::currentConfig->appMain->encoding)
	{
		QuickMessage(bonkEnc::i18n->TranslateString("Cannot modify the joblist while encoding!"), bonkEnc::i18n->TranslateString("Error"), MB_OK, IDI_HAND);

		return False;
	}

	if (bonkEnc::currentConfig->appMain->playing) bonkEnc::currentConfig->appMain->StopPlayback();

	for (Int i = 0; i < GetNOfTracks(); i++) delete GetNthTrack(i);

	tracks.RemoveAll();

	Clear();

	text->SetText(bonkEnc::i18n->TranslateString("%1 file(s) in joblist:").Replace("%1", "0"));

	((bonkEncGUI *) bonkEnc::currentConfig->appMain)->ClearEditFields();

	return True;
}

Track *BonkEnc::JobList::GetSelectedTrack()
{
	ListEntry	*entry = GetSelectedEntry();
	Int		 n = 0;

	for (Int i = 0; i < GetNOfEntries(); i++)
	{
		if (GetNthEntry(i) == entry) n = i;
	}

	return GetNthTrack(n);
}

Int BonkEnc::JobList::SetMetrics(Point nPos, Size nSize)
{
	droparea->SetMetrics(nPos, nSize);

	return ListBox::SetMetrics(nPos, nSize);
}

Void BonkEnc::JobList::AddTrackByDialog()
{
	if (bonkEnc::currentConfig->appMain->encoding)
	{
		QuickMessage(bonkEnc::i18n->TranslateString("Cannot modify the joblist while encoding!"), bonkEnc::i18n->TranslateString("Error"), MB_OK, IDI_HAND);

		return;
	}

	FileSelection	*dialog = new FileSelection();

	dialog->SetParentWindow(container->GetContainerWindow());
	dialog->SetFlags(SFD_ALLOWMULTISELECT);

	Array<String>	 types;
	Array<String>	 extensions;

	for (Int i = 0; i < bonkEncDLLInterfaces::winamp_in_plugins.GetNOfEntries(); i++)
	{
		Int		 n = 1;
		Int		 k = 0;
		String		 type;
		String		 extension;

		for (Int j = 0; true; j++)
		{
			if (!(n & 1))
			{
				type[k++] = bonkEncDLLInterfaces::winamp_in_modules.GetNthEntry(i)->FileExtensions[j];

				if (bonkEncDLLInterfaces::winamp_in_modules.GetNthEntry(i)->FileExtensions[j] == 0)
				{
					types.AddEntry(type);

					k = 0;
					n++;
					type = "";
				}
			}
			else
			{
				extension[k++] = bonkEncDLLInterfaces::winamp_in_modules.GetNthEntry(i)->FileExtensions[j];

				if (bonkEncDLLInterfaces::winamp_in_modules.GetNthEntry(i)->FileExtensions[j] == 0)
				{
					String	 extension2 = String("*.").Append(extension);
					Int	 o = 0;		

					for (Int m = 0; m < extension2.Length(); m++)
					{
						extension[m + o] = extension2[m];

						if (extension2[m] == ';')
						{
							extension[m + o + 1] = ' ';
							extension[m + o + 2] = '*';
							extension[m + o + 3] = '.';
							o += 3;
						}
					}

					if (extension != "*.AAC") extensions.AddEntry(extension);

					k = 0;
					n++;
					extension = "";
				}
			}

			if (bonkEncDLLInterfaces::winamp_in_modules.GetNthEntry(i)->FileExtensions[j] == 0 && bonkEncDLLInterfaces::winamp_in_modules.GetNthEntry(i)->FileExtensions[j + 1] == 0) break;
		}
	}

	String	 fileTypes;

	if (bonkEnc::currentConfig->enable_faad2)							fileTypes.Append("*.aac; ");
													fileTypes.Append("*.aif; *.aiff; *.au");
	if (bonkEnc::currentConfig->enable_bonk)							fileTypes.Append("; *.bonk");
	if (bonkEnc::currentConfig->enable_cdrip && bonkEnc::currentConfig->cdrip_numdrives >= 1)	fileTypes.Append("; *.cda");
	if (bonkEnc::currentConfig->enable_flac)							fileTypes.Append("; *.flac");
	if (bonkEnc::currentConfig->enable_lame)							fileTypes.Append("; *.mp3");
	if (bonkEnc::currentConfig->enable_mp4 && bonkEnc::currentConfig->enable_faad2)			fileTypes.Append("; *.m4a; *.m4b; *.mp4");
	if (bonkEnc::currentConfig->enable_vorbis)							fileTypes.Append("; *.ogg");
													fileTypes.Append("; *.voc; *.wav");

	for (Int l = 0; l < extensions.GetNOfEntries(); l++) fileTypes.Append("; ").Append(extensions.GetNthEntry(l));

													dialog->AddFilter(bonkEnc::i18n->TranslateString("Audio Files"), fileTypes);
	if (bonkEnc::currentConfig->enable_faad2)							dialog->AddFilter(bonkEnc::i18n->TranslateString("AAC Files").Append(" (*.aac)"), "*.aac");
													dialog->AddFilter(bonkEnc::i18n->TranslateString("Apple Audio Files").Append(" (*.aif; *.aiff)"), "*.aif; *.aiff");
	if (bonkEnc::currentConfig->enable_bonk)							dialog->AddFilter(bonkEnc::i18n->TranslateString("Bonk Files").Append(" (*.bonk)"), "*.bonk");
													dialog->AddFilter(bonkEnc::i18n->TranslateString("Creative Voice Files").Append(" (*.voc)"), "*.voc");
	if (bonkEnc::currentConfig->enable_flac)							dialog->AddFilter(bonkEnc::i18n->TranslateString("FLAC Files").Append(" (*.flac)"), "*.flac");
	if (bonkEnc::currentConfig->enable_lame)							dialog->AddFilter(bonkEnc::i18n->TranslateString("MP3 Files").Append(" (*.mp3)"), "*.mp3");
	if (bonkEnc::currentConfig->enable_mp4 && bonkEnc::currentConfig->enable_faad2)			dialog->AddFilter(bonkEnc::i18n->TranslateString("MP4 Files").Append(" (*.m4a; *.m4b; *.mp4)"), "*.m4a; *.m4b; *.mp4");
	if (bonkEnc::currentConfig->enable_vorbis)							dialog->AddFilter(bonkEnc::i18n->TranslateString("Ogg Vorbis Files").Append(" (*.ogg)"), "*.ogg");
													dialog->AddFilter(bonkEnc::i18n->TranslateString("Sun Audio Files").Append(" (*.au)"), "*.au");
													dialog->AddFilter(bonkEnc::i18n->TranslateString("Wave Files").Append(" (*.wav)"), "*.wav");
	if (bonkEnc::currentConfig->enable_cdrip && bonkEnc::currentConfig->cdrip_numdrives >= 1)	dialog->AddFilter(bonkEnc::i18n->TranslateString("Windows CD Audio Track").Append(" (*.cda)"), "*.cda");

	for (Int m = 0; m < types.GetNOfEntries(); m++) dialog->AddFilter(types.GetNthEntry(m), extensions.GetNthEntry(m));

	dialog->AddFilter(bonkEnc::i18n->TranslateString("All Files"), "*.*");

	if (dialog->ShowDialog() == Success)
	{
		for (int i = 0; i < dialog->GetNumberOfFiles(); i++)
		{
			String	 file = dialog->GetNthFileName(i);

			AddTrackByFileName(file);

			bonkEnc::currentConfig->appMain->cddbRetry = False;
		}

		bonkEnc::currentConfig->appMain->cddbRetry = True;
	}

	delete dialog;
}

Void BonkEnc::JobList::AddTrackByFileName(String file, String outfile)
{
	if (bonkEnc::currentConfig->appMain->encoding)
	{
		QuickMessage(bonkEnc::i18n->TranslateString("Cannot modify the joblist while encoding!"), bonkEnc::i18n->TranslateString("Error"), MB_OK, IDI_HAND);

		return;
	}

	Track	*format = NIL;

	if (file.CompareN("/cda", 4) == 0)
	{
		InputFilter	*filter_in = new FilterInCDRip(bonkEnc::currentConfig, NIL);

		format = filter_in->GetFileInfo(file);

		delete filter_in;
	}
	else
	{
		InputFilter	*filter_in = Utilities::CreateInputFilter(file, NIL);

		if (filter_in != NIL)
		{
			format = filter_in->GetFileInfo(file);

			delete filter_in;
		}
	}

	if (format == NIL)
	{
		QuickMessage(bonkEnc::i18n->TranslateString("Cannot open file:").Append(" ").Append(file), bonkEnc::i18n->TranslateString("Error"), MB_OK, IDI_HAND);

		return;
	}
	else if (format->rate == 0 || format->channels == 0)
	{
		QuickMessage(bonkEnc::i18n->TranslateString("Cannot open file:").Append(" ").Append(file), bonkEnc::i18n->TranslateString("Error"), MB_OK, IDI_HAND);

		return;
	}

	if (format->artist == NIL && format->title == NIL)
	{
		if (file.CompareN("/cda", 4) != 0)
		{
			String	 fileName;
			Int	 in_len = file.Length();
			Int	 lastBs = -1;
			Int	 firstDot = 0;

			for (Int i = 0; i < in_len; i++)
			{
				if (file[i] == '\\') lastBs = i;
			}

			for (Int j = in_len - 1; j >= 0; j--)
			{
				if (file[j] == '.') { firstDot = in_len - j; break; }
				if (file[j] == '\\') break;
			}

			for (Int k = 0; k < (in_len - lastBs - firstDot - 1); k++)
			{
				fileName[k] = file[k + lastBs + 1];
			}

			Bool	 goodFormat = False;

			for (Int l = 1; l < fileName.Length() - 1; l++)
			{
				if (fileName[l - 1] == ' ' &&
				    fileName[  l  ] == '-' &&
				    fileName[l + 1] == ' ')
				{
					goodFormat = True;

					break;
				}
			}

			if (goodFormat)
			{
				Int	 artistComplete = 0;
				Int	 m = 0;

				for (m = 0; m < fileName.Length(); m++)
				{
					if (fileName[  m  ] == ' ' &&
					    fileName[m + 1] == '-' &&
					    fileName[m + 2] == ' ')
					{
						artistComplete = m + 3;

						m += 3;
					}

					if (!artistComplete)	format->artist[m] = fileName[m];
					else			format->title[m - artistComplete] = fileName[m];
				}

				format->title[m - artistComplete] = 0;
			}
		}
	}

	if (format->fileSize > 0) format->fileSizeString = Utilities::LocalizeNumber(format->fileSize);

	if (format->length > 0)	format->lengthString = String::FromInt(Math::Floor(format->length / (format->rate * format->channels) / 60)).Append(":").Append((format->length / (format->rate * format->channels) % 60) < 10 ? "0" : "").Append(String::FromInt(format->length / (format->rate * format->channels) % 60));
	else			format->lengthString = "?";

	if (format->origFilename == NIL) format->origFilename = file;

	format->outfile = outfile;

	AddTrack(format);
	Paint(SP_UPDATE);

	text->SetText(bonkEnc::i18n->TranslateString("%1 file(s) in joblist:").Replace("%1", String::FromInt(GetNOfTracks())));
}

Void BonkEnc::JobList::AddTrackByDragAndDrop(const String &file)
{
	AddTrackByFileName(file);
}

Void BonkEnc::JobList::RemoveSelectedTrack()
{
	if (bonkEnc::currentConfig->appMain->encoding)
	{
		QuickMessage(bonkEnc::i18n->TranslateString("Cannot modify the joblist while encoding!"), bonkEnc::i18n->TranslateString("Error"), MB_OK, IDI_HAND);

		return;
	}

	if (GetSelectedEntry() == NIL)
	{
		QuickMessage(bonkEnc::i18n->TranslateString("You have not selected a file!"), bonkEnc::i18n->TranslateString("Error"), MB_OK, IDI_HAND);

		return;
	}

	ListEntry	*entry = GetSelectedEntry();
	Int		 n = 0;

	for (Int i = 0; i < GetNOfEntries(); i++)
	{
		if (GetNthEntry(i) == entry) n = i;
	}

	if (bonkEnc::currentConfig->appMain->playing && bonkEnc::currentConfig->appMain->player_entry == n) bonkEnc::currentConfig->appMain->StopPlayback();
	if (bonkEnc::currentConfig->appMain->playing && bonkEnc::currentConfig->appMain->player_entry > n) bonkEnc::currentConfig->appMain->player_entry--;

	Surface	*surface = container->GetDrawSurface();
	Point	 realPos = GetRealPosition();
	Rect	 frame;

	frame.left	= realPos.x;
	frame.top	= realPos.y;
	frame.right	= realPos.x + size.cx - 1;
	frame.bottom	= realPos.y + size.cy - 1;

	surface->StartPaint(frame);

	RemoveNthTrack(n);

	if (GetNOfEntries() > 0)
	{
		if (n < GetNOfEntries())	SelectEntry(GetNthEntry(n));
		else				SelectEntry(GetNthEntry(n - 1));
	}

	surface->EndPaint();

	text->SetText(bonkEnc::i18n->TranslateString("%1 file(s) in joblist:").Replace("%1", String::FromInt(GetNOfEntries())));

	if (GetNOfEntries() > 0)	((bonkEncGUI *) bonkEnc::currentConfig->appMain)->SelectJoblistEntry();
	else				((bonkEncGUI *) bonkEnc::currentConfig->appMain)->ClearEditFields();
}


Void BonkEnc::JobList::SelectAll()
{
	for (Int i = 0; i < GetNOfEntries(); i++)
	{
		if (!GetNthEntry(i)->IsMarked()) GetNthEntry(i)->SetMark(True);
	}
}

Void BonkEnc::JobList::SelectNone()
{
	for (Int i = 0; i < GetNOfEntries(); i++)
	{
		if (GetNthEntry(i)->IsMarked()) GetNthEntry(i)->SetMark(False);
	}
}

Void BonkEnc::JobList::ToggleSelection()
{
	for (Int i = 0; i < GetNOfEntries(); i++)
	{
		if (GetNthEntry(i)->IsMarked())	GetNthEntry(i)->SetMark(False);
		else				GetNthEntry(i)->SetMark(True);
	}
}

Void BonkEnc::JobList::LoadList()
{
}

Void BonkEnc::JobList::SaveList()
{
}

Void BonkEnc::JobList::OnRegister(Container *container)
{
	container->RegisterObject(droparea);
	container->RegisterObject(text);

	container->RegisterObject(button_sel_all);
	container->RegisterObject(button_sel_none);
	container->RegisterObject(button_sel_toggle);

	((bonkEncGUI *) bonkEnc::currentConfig->appMain)->onChangeLanguageSettings.Connect(&JobList::OnChangeLanguageSettings, this);
}

Void BonkEnc::JobList::OnUnregister(Container *container)
{
	container->UnregisterObject(droparea);
	container->UnregisterObject(text);

	container->UnregisterObject(button_sel_all);
	container->UnregisterObject(button_sel_none);
	container->UnregisterObject(button_sel_toggle);

	((bonkEncGUI *) bonkEnc::currentConfig->appMain)->onChangeLanguageSettings.Disconnect(&JobList::OnChangeLanguageSettings, this);
}

Void BonkEnc::JobList::OnChangeLanguageSettings()
{
	text->SetText(bonkEnc::i18n->TranslateString("%1 file(s) in joblist:").Replace("%1", String::FromInt(GetNOfEntries())));

	button_sel_all->SetTooltipText(bonkEnc::i18n->TranslateString("Select all"));
	button_sel_none->SetTooltipText(bonkEnc::i18n->TranslateString("Select none"));
	button_sel_toggle->SetTooltipText(bonkEnc::i18n->TranslateString("Toggle selection"));
}
