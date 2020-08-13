package com.example.echo

import android.os.Bundle
import androidx.annotation.Keep
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_tick.*

/**
 * Created by  On 2020/7/9
 */
class TicksActivity : AppCompatActivity() {
    var hour = 0
    var minute = 0
    var second = 0
    companion object{
        init {
            System.loadLibrary("hello-jnicallback")
        }
    }
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_tick)

    }
    override fun onResume() {
        super.onResume()
        second = 0
        minute = second
        hour = minute
        startTicks()
    }

    @Keep
    private fun updateTimer() {
        ++second
        if (second >= 60) {
            ++minute
            second -= 60
            if (minute >= 60) {
                ++hour
                minute -= 60
            }
        }
        runOnUiThread {
            val ticks = "" + this.hour + ":" +
                    this.minute + ":" +
                    this@TicksActivity.second
            this@TicksActivity.tickView.setText(ticks)
        }
    }
    external fun startTicks()
    external fun StopTicks()
}