#!/usr/bin/perl

use Cwd;
use LWP::Simple;
use File::Basename;
use File::Spec::Functions qw(rel2abs);

# Updates the default Valve Source engined games.

sub update_game;

$BASE_FOLDER=dirname(rel2abs($0)) . "/..";

if ($^O eq "MSWin32")
{
	print "Windows platform\n";
	$ROOT_PATH=$BASE_FOLDER;
	$UPDATER="HldsUpdateTool.exe";
}
else
{
#Linux platform
	print "Linux platform\n";
	$ROOT_PATH=$BASE_FOLDER;
	$UPDATER="./steam";	
	$LINUX="TRUE";
}

$SRCDS_PATH="$ROOT_PATH/srcds_1";

#Add new games here with their Valve game names
%game_cmd = (
"Counter-Strike Source"		=> "232330",
"Half-Life 2 Deathmatch" 	=> "232370",
"Team Fortress 2" 		=> "232250",
"Day of Defeat Source" 		=> "232290"
#"Left 4 Dead 2" 			=> "222860",
);


print "\n";

$count=1;
for my $key ( sort keys %game_cmd ) 
{
	my $value = $game_cmd{$key};
        print "$count - $key\n";
        $count = $count + 1;
}

print "\nChoose a game to update or type \"all\" to update all games\n\n";

$question_response = <>;
chomp($question_response);
chdir($SRCDS_PATH);

my $dir = getcwd();

print "Current path = $dir\n";

if ($question_response eq "all")
{
	for my $key ( sort keys %game_cmd ) 
	{
		my $value = $game_cmd{$key};
		update_game($value);
	}
}
else
{
	$count=1;
	for my $key ( sort keys %game_cmd ) 
	{
		if ($count eq $question_response)
		{
			my $value = $game_cmd{$key};
			update_game($value);
		}	        
	        $count = $count + 1;
	}	
}



sub update_game
{
	if (! -e $UPDATER)
	{
		# No installation found
		if ($LINUX)
		{
			# Download the SteamCMD tool and run it
			system("wget http://media.steampowered.com/client/steamcmd_linux.tar.gz");
			system(tar -xvzf steamcmd_linux.tar.gz)
			system("chmod +x steamcmd.sh");
			system("./steamcmd.sh");
		}
		else
		{
			# Windows 
			print "Getting HldsUpdateTool from Valve...\n";
			print "\n\n\nWhen the popup appears, install to " . $SRCDS_PATH. "\n";

			getstore("http://www.steampowered.com/download/hldsupdatetool.exe", "HldsUpdateToolDeleteMe.exe");

			# Run the windows installer    
			system("HldsUpdateToolDeleteMe.exe");

			# Delete the downloaded file
			if (-e "HldsUpdateToolDeleteMe.exe")
			{
				unlink("HldsUpdateToolDeleteMe.exe");
			}
		}
	}

	print "\nPerl script - Updating Game $_[0]\n";
	$system_command="$UPDATER -command update -game \"$_[0]\" -dir $SRCDS_PATH";
	system($system_command);
}

