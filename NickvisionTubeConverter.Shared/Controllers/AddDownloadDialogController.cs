using NickvisionTubeConverter.Shared.Helpers;
using NickvisionTubeConverter.Shared.Models;
using System.Collections.Generic;
using System.IO;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace NickvisionTubeConverter.Shared.Controllers;

/// <summary>
/// Statuses for when a download is checked
/// </summary>
public enum DownloadCheckStatus
{
    Valid = 1,
    EmptyVideoUrl = 2,
    InvalidVideoUrl = 4,
    InvalidSaveFolder = 8,
    PlaylistNotSupported = 16
}

/// <summary>
/// A controller for a AddDownloadDialog
/// </summary>
public class AddDownloadDialogController
{
    private string? _previousUrl;
    private string _saveFolder;
    private string _saveFilename;

    /// <summary>
    /// The localizer to get translated strings from
    /// </summary>
    public Localizer Localizer { get; init; }
    /// <summary>
    /// The downloads created by the dialog
    /// </summary>
    public List<Download> Downloads { get; init; }
    /// <summary>
    /// Whether or not the dialog was accepted (response)
    /// </summary>
    public bool Accepted { get; set; }

    /// <summary>
    /// The previously used save folder
    /// </summary>
    public string PreviousSaveFolder => Configuration.Current.PreviousSaveFolder;
    /// <summary>
    /// The previously used MediaFileType
    /// </summary>
    public MediaFileType PreviousMediaFileType => Configuration.Current.PreviousMediaFileType;
    /// <summary>
    /// The save path
    /// </summary>
    public string SavePath => $"{_saveFolder}{Path.DirectorySeparatorChar}{_saveFilename}";

    /// <summary>
    /// Constructs a AddDownloadDialogController
    /// </summary>
    public AddDownloadDialogController(Localizer localizer)
    {
        _previousUrl = null;
        _saveFolder = "";
        _saveFilename = "";
        Localizer = localizer;
        Downloads = new List<Download>();
        Accepted = false;
    }

    /// <summary>
    /// Updates the Download object
    /// </summary>
    /// <param name="videoUrl">The url of the video to download</param>
    /// <param name="mediaFileType">The file type to download the video as</param>
    /// <param name="savePath">The path to save the download to</param>
    /// <param name="quality">The quality of the download</param>
    /// <param name="subtitles">The subtitles for the download</param>
    /// <returns>The DownloadCheckStatus</returns>
    public async Task<DownloadCheckStatus> UpdateDownloadAsync(string videoUrl, MediaFileType mediaFileType, string savePath, Quality quality, Subtitle subtitles)
    {
        DownloadCheckStatus result = 0;
        if (string.IsNullOrEmpty(savePath))
        {
            _saveFolder = PreviousSaveFolder;
            if (!Directory.Exists(_saveFolder))
            {
                result |= DownloadCheckStatus.InvalidSaveFolder;
            }
        }
        else
        {
            _saveFolder = Path.GetDirectoryName(savePath)!;
            _saveFilename = Path.GetFileNameWithoutExtension(savePath) + mediaFileType.GetDotExtension();
        }
        if (string.IsNullOrEmpty(videoUrl))
        {
            result |= DownloadCheckStatus.EmptyVideoUrl;
        }
        else if (_previousUrl != videoUrl)
        {
            _previousUrl = videoUrl;
            var videoUrlInfo = await VideoUrlInfo.GetAsync(videoUrl);
            if (videoUrlInfo == null)
            {
                result |= DownloadCheckStatus.InvalidVideoUrl;
            }
            else
            {
                if (videoUrlInfo.IsPlaylist)
                {
                    result |= DownloadCheckStatus.PlaylistNotSupported;
                }
                else if (videoUrlInfo.IsSingleVideo)
                {
                    _saveFilename = videoUrlInfo.SingleTitle + mediaFileType.GetDotExtension();
                }
            }
        }
        if (result != 0)
        {
            return result;
        }
        Downloads.Add(new Download(videoUrl, mediaFileType, _saveFolder, _saveFilename, quality, subtitles));
        if (!string.IsNullOrEmpty(_saveFolder) && !Regex.Match(_saveFolder, @"^\/run\/user\/.*\/doc\/.*").Success)
        {
            Configuration.Current.PreviousSaveFolder = _saveFolder;
        }
        Configuration.Current.PreviousMediaFileType = mediaFileType;
        Configuration.Current.Save();
        return DownloadCheckStatus.Valid;
    }
}
