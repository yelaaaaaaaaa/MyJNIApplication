package com.example.echo

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity

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
    external fun startTicks()
    external fun StopTicks()
}