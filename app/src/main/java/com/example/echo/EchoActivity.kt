package com.example.echo

import android.Manifest
import android.app.Activity
import android.content.Context
import android.content.pm.PackageManager
import android.media.AudioFormat
import android.media.AudioManager
import android.media.AudioRecord
import android.os.Bundle
import android.view.View
import android.widget.SeekBar
import android.widget.SeekBar.OnSeekBarChangeListener
import android.widget.TextView
import androidx.core.app.ActivityCompat
import androidx.core.app.ActivityCompat.OnRequestPermissionsResultCallback
import kotlinx.android.synthetic.main.activity_echo.*

/**
 * Created by  On 2020/6/30
 */
class EchoActivity : Activity(), OnRequestPermissionsResultCallback{
    private val AUDIO_ECHO_REQUEST = 0

    private var nativeSampleRate: String? = null
    private var nativeSampleBufSize: String? = null

    //private var delaySeekBar: SeekBar? = null
    private var echoDelayProgress = 0


    private var echoDecayProgress = 0f

    private var supportRecording = false
    private var isPlaying = false
    companion object {
        init {
            System.loadLibrary("echo")
        }
    }
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_echo)
        queryNativeAudioParameters()

       // delaySeekBar = findViewById<View>(R.id.delaySeekBar) as SeekBar
        echoDelayProgress = delaySeekBar.progress * 1000 / delaySeekBar.max
        delaySeekBar.setOnSeekBarChangeListener(object : OnSeekBarChangeListener {
            override fun onProgressChanged(
                seekBar: SeekBar,
                progress: Int,
                fromUser: Boolean
            ) {
                val curVal = progress.toFloat() / delaySeekBar!!.max
                curDelay!!.text = String.format("%s", curVal)
                setSeekBarPromptPosition(delaySeekBar, curDelay)
                if (!fromUser) return
                echoDelayProgress = progress * 1000 / delaySeekBar.max
                configureEcho(echoDelayProgress, echoDecayProgress)
            }

            override fun onStartTrackingTouch(seekBar: SeekBar) {}
            override fun onStopTrackingTouch(seekBar: SeekBar) {}
        })
        delaySeekBar.post(Runnable { setSeekBarPromptPosition(delaySeekBar, curDelay) })

        echoDecayProgress = decaySeekBar.progress.toFloat() / decaySeekBar.max
        decaySeekBar.setOnSeekBarChangeListener(object : OnSeekBarChangeListener {
            override fun onProgressChanged(
                seekBar: SeekBar,
                progress: Int,
                fromUser: Boolean
            ) {
                val curVal = progress.toFloat() / seekBar.max
                curDelay.text = String.format("%s", curVal)
                setSeekBarPromptPosition(decaySeekBar, curDelay)
                if (!fromUser) return
                echoDecayProgress = curVal
                configureEcho(echoDelayProgress, echoDecayProgress)
            }

            override fun onStartTrackingTouch(seekBar: SeekBar) {}
            override fun onStopTrackingTouch(seekBar: SeekBar) {}
        })
        decaySeekBar.post(Runnable { setSeekBarPromptPosition(decaySeekBar, curDelay) })

        // initialize native audio system

        // initialize native audio system
        updateNativeAudioUI()

        if (supportRecording) {
           createSLEngine(
                nativeSampleRate!!.toInt(), nativeSampleBufSize!!.toInt(),
                echoDelayProgress.toLong(),
                echoDecayProgress
            )
        }
    }



    private fun updateNativeAudioUI() {
        if (!supportRecording) {
            statusView.text = getString(R.string.mic_error_msg)
            capture_control_button.isEnabled = false
            return
        }

        statusView.text = getString(
            R.string.fast_audio_info_msg,
            nativeSampleRate, nativeSampleBufSize
        )
    }

    private fun setSeekBarPromptPosition(seekBar: SeekBar, label: TextView) {
        val thumbX = seekBar.progress.toFloat() / seekBar.max *
                seekBar.width + seekBar.x
        label.x = thumbX - label.width / 2.0f
    }
    private fun queryNativeAudioParameters() {
        supportRecording = true
        val myAudioMgr =
            getSystemService(Context.AUDIO_SERVICE) as AudioManager
        if (myAudioMgr == null) {
            supportRecording = false
            return
        }
        nativeSampleRate = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE)
        nativeSampleBufSize = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER)
        // hardcoded channel to mono: both sides -- C++ and Java sides

        // hardcoded channel to mono: both sides -- C++ and Java sides
        val  nativeSampleRateCopy = nativeSampleRate
        val recBufSize = AudioRecord.getMinBufferSize(
            nativeSampleRateCopy!!.toInt(),
            AudioFormat.CHANNEL_IN_MONO,
            AudioFormat.ENCODING_PCM_16BIT
        )
        if (recBufSize == AudioRecord.ERROR ||
            recBufSize == AudioRecord.ERROR_BAD_VALUE
        ) {
            supportRecording = false
        }
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
    }




    fun onEchoClick(view: View) {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO) !=
            PackageManager.PERMISSION_GRANTED
        ) {
            statusView.text = getString(R.string.request_permission_status_msg)
            ActivityCompat.requestPermissions(
                this, arrayOf(Manifest.permission.RECORD_AUDIO),
                this.AUDIO_ECHO_REQUEST
            )
            return
        }
        startEcho()
    }

    private fun startEcho() {
        if (!supportRecording) {
            return
        }
        if (!isPlaying) {
            if (!createSLBufferQueueAudioPlayer()) {
                statusView.text = getString(R.string.player_error_msg)
                return
            }
            if (!createAudioRecorder()) {
                deleteSLBufferQueueAudioPlayer()
                statusView.text = getString(R.string.recorder_error_msg)
                return
            }
            startPlay() // startPlay() triggers startRecording()
            statusView.text = getString(R.string.echoing_status_msg)
        } else {
            stopPlay() // stopPlay() triggers stopRecording()
            updateNativeAudioUI()
            deleteAudioRecorder()
           deleteSLBufferQueueAudioPlayer()
        }
        isPlaying = !isPlaying
        capture_control_button.setText(getString(if (isPlaying) R.string.cmd_stop_echo else R.string.cmd_start_echo))
    }

    fun getLowLatencyParameters(view: View) {
        updateNativeAudioUI()
    }

    external fun configureEcho(delayInMs: Int, decay: Float): Boolean
    external fun createSLEngine(
        rate: Int, framesPerBuf: Int,
        delayInMs: Long, decay: Float
    )
    external fun createSLBufferQueueAudioPlayer() :Boolean
    external fun createAudioRecorder(): Boolean
    external fun deleteAudioRecorder()
    external fun startPlay()
    external fun stopPlay()
    external fun deleteSLBufferQueueAudioPlayer()
}